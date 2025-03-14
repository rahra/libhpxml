# Libhpxml – A High Performance XML Stream Parser

Libhpxml is a high performance XML stream parser library written in C with a simple API. 

## Description

Libhpxml is a high performance XML stream parser library written in C with a
simple API. It is intended to parse XML files very speed efficient. This may be
required when processing huge XML files like the OSM planet file which
currently has 260GB+.
The development goals are **speed efficiency** and **simple memory handling** to
reduce the risk of memory leaks. These objectives are achieved through

* avoidance of system calls (such as `malloc(3)`) as much as possible,
* usage of (nearly) static memory buffers, and
* avoidance of copying memory.

Being a stream parser, libhpxml returns in a loop one XML element after the
other. It uses a result buffer which is initialized once and then reused for
every element. Thus, repeated calls to `malloc(3)` and `free(3)` are omitted. The
input data is read in blocks. The result buffer does not contain the data
itself but just pointers to the XML elements within the input buffer. Thus,
data is not copied, it is just pointed to.

## Usage

### Initialization

libhpxml provides a set of functions and structures. `hpx_ctrl_t` is a control
structure which contains all relevant information for a XML stream. The
contents of the structure are used internally by the library and should not be
modified in any way. The structure is initialized with a call to `hpx_init()` and
must be freed again with `hpx_free()`. Note that `hpx_free()` will not close the
file descriptor.

```C
   hpx_ctrl_t *hpx_init(int fd, int len);
   void hpx_free(hpx_ctrl_t *ctl);
```

The arguments to `hpx_init()` is a file descriptor to an open XML file and the
length of the block buffer. It will initialize a `hpx_ctrl_t` structure and
returns a pointer to it. In case of error NULL is returned and errno is set
appropriately. The buffer size must be at least as large as the longest XML
element in the file but it is recommended to be much larger. The larger the
buffer the lesser the number of reads. If there is enough system memory
available, it is safe to choose 100MB or even more.

### Memory Mapping

Libhpxml now supports memory mapping through the system call `mmap(2)`. This is
activated if `hpx_init()` is called with a negative len parameter. In case of
memory mapping, len must be as long as the (negative value) of the total length
of the XML input file. Memory mapping of files greater than 2 GB is currently
just supported on 64 bit architectures (see manpage `mmap(2)` or POSIX manpage
`mmap(3)`, respectively).

The main application for memory mapping is if libhpxml is not just used as
stream parser but XML objects are kept in memory during the whole runtime. This
is necessary if on-the-fly object processing is not possible. This typically is
the case if XML objects are nested or they depend on each other. An example is
the rendering process of OSM data.
Keeping pointers valid (see `hpx_get_elem()`) is still possible without memory
mapping, but it requires that the buffer is as large as the file itself because
it has to pull in the whole file at once. Thus, this works just if the system
has enough memory. Memory mapping in contrast does not require physical memory,
hence, even a file with several hundred GB may be used.
Note that the preprocessor macro `WITH_MMAP` must be defined at compile time to
compile libhpxml with `mmap(2)` support. If it was not compiled with `WITH_MMAP`,
`hpx_init()` will fail, in which case `NULL` is returned and `errno` is set to
`EINVAL`.

### Supporting Functions

While parsing an XML file libhpxml returns pointers to the elements and
attributes. C strings are usually '\0'-terminated but this is not applicable
here because it would require that '\0' characters are inserted after each
element, resulting in huge data movement. Thus, libhpxml uses "B strings" which
are hold in the `bstring_t` structure. The structure contains a pointer to the
string and its length. Additionally, a set of function is provided to handle
those strings.

```C
   typedef struct bstring
   {
      int len;
      char *buf;
   } bstring_t;
```

### Processing Elements

After initializing the control structure, XML elements are subsequently
retrieved by repeated calls to hpx_get_elem().

```C
   int hpx_get_elem(hpx_ctrl_t *ctl, bstring_t *b, int *in_tag, size_t *lno);
```

The function processes the buffer and fills out the bstring pointing to the
next XML element. ctl is the pointer to control structure. `in_tag` is filled
with either 0 or 1, either if the XML element is a tag (<...>) or if it is
literal text between tags. lno is filled with the line number at which this
element starts. Both, `in_tag` and `lno` may be `NULL` if it is not used.
`hpx_get_elem()` returns the length of the bstring, 0 on EOF, and -1 in case of
error. Such an element can now be parsed with a call to `hpx_process_elem()`.

```C
   int hpx_process_elem(bstring_t b, hpx_tag_t *p);

   typedef struct hpx_tag
   {
      bstring_t tag;       // name of tag
      int type;            // type of tag
      int nattr;           // number of attributes
      int mattr;           // size of attr array
      hpx_attr_t attr[];   // array containing attributes
   } hpx_tag_t;

   typedef struct hpx_attr
   {
      bstring_t name;   //! name of attribute
      bstring_t value;  //! value of attribute
      char delim;       //! delimiter character of attribute value
   } hpx_attr_t;
```

It takes a bstring which contains an XML element and parses it into the
`hpx_tag_t` structure. This structure may be initialized using `hpx_tm_create()`
but it may also be initialized manually. In the latter case the structure
member mattr must contain the size of the attribute array. Otherwise the
program may segfault. The argument to `hpx_tm_create()` is the maximum number of
expected attributes. The tag structure should be freed again with `hpx_tm_free()`
after use. It is recommended to reuse the tag structure. This reduces
unnecessary memory management system calls.
Please note that a call to `hpx_get_elem()` may invalidate the pointers within
previously filled-out tag structures because it might read in the next block of
the input file. Thus, the tag must be processed before the next call to
`hpx_get_elem()`.
The type member of `hpx_tag_t` defines the type this XML element. Currently,
the following types are known.

enum | Description | Example
---- | ----------- | -------
`HPX_ILL`  | Element unknown. This may indicate a syntax error.
`HPX_OPEN` | An XML opening tag.  `<tagname attrname="attrval"...>`
`HPX_SINGLE` | A single, closed XML tag.  `<tagname attrname="attrval".../>`
`HPX_CLOSE` |  An XML closing tag.  `</tagname>`
`HPX_LITERAL` | No tag, just text between tags.  
`HPX_ATT` | Declarations.  `<! ..... >`
`HPX_INSTR` | Instructions.  `<? .... ?>`
`HPX_COMMENT` | Comments.   `<!-- .... -->`
`HPX_CDATA` | CDATA.  `<![CDATA[ .... ]]>`

```C
   hpx_tag_t *hpx_tm_create(int n);
   void hpx_tm_free(hpx_tag_t *t);
```

The tag structure further contains an array of attributes. The member nattr
contains the actual number of attributes parsed. It is always at most mattr
elements. If an XML tag has more than mattr elements they are just ignored. At
the current version there's no feedback to the calling function. This will be
improved in future releases. The attributes themselves are stored each in an
`hpx_attr_t` structure. It contains two bstrings, one for the name and one for
the value of the attribute. The third member delim keeps the delimiter of the
value which is either `'\''` (single quote, 0x27) or `'"'` (double quote, 0x22).

## Example

This example parses an XML file and outputs some stats about each XML element.
You can download the example directly here.

```C
#include <stdio.h>
#include <stdlib.h>
 
#include "bstring.h"
#include "libhpxml.h"
 
 
int main(int argc, char *argv[])
{
   hpx_ctrl_t *ctl;
   hpx_tag_t *tag;
   bstring_t b;
   size_t lno;
 
   // initialize control structure, stdin, 100MB buffer
   if ((ctl = hpx_init(0, 100*1024*1024)) == NULL)
      perror("hpx_init_simple"), exit(EXIT_FAILURE);
   // initialize tag structure with maximum 16 attributes
   if ((tag = hpx_tm_create(16)) == NULL)
      perror("hpx_tm_create"), exit(EXIT_FAILURE);
 
   // loop as long as XML elements are available
   while (hpx_get_elem(ctl, &b, NULL, &lno) > 0)
   {
      // parse XML element
      if (!hpx_process_elem(b, tag))
      {
         // element successfully parsed, do something with it
         // ...
         // ...
 
         printf("[%ld] type=%d, name=%.*s, nattr=%d\n", lno, tag->type, tag->tag.len, tag->tag.buf, tag->nattr);
      }
      else
         printf("[%ld] ERROR in element: %.*s\n", lno, b.len, b.buf);
   }
 
   if (!ctl->eof)
      perror("hpx_get_elem"), exit(EXIT_FAILURE);
 
   hpx_tm_free(tag);
   hpx_free(ctl);
 
   exit(EXIT_SUCCESS);
}
```
 

## Bugs and Caveats

libhpxml does not validate the XML file using e.g. DTD. Thus, it does not care
about semantic errors. Syntactical ones of course are reported. In the current
version, libhpxml is not thread-safe. The interface to the functions may change
because it is in early development. The array of attributes within the
`hpx_tag_t` structure has a static size and is not resized if an XML tag has more
attributes as array entries are available. Currently, `hpx_process_elem()` does
not report if the number of attributes would exceed the array (of course, it
does not exhaust it).

## Author

libhpxml is developed and maintained by Bernhard R. Fischer, 4096R/8E24F29D <bf@abenteuerland.at>.

## License

Libhpxml is released under GNU GPLv3. 

