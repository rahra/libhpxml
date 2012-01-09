<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
"http://www.w3.org/TR/html4/strict.dtd">
<html>
   <head>
      <meta name="author" content="Bernhard R. Fischer">
      <meta name="date" content="2012-01-09T12:06:00+0200">
      <meta http-equiv="content-type" content="text/html; charset=UTF-8">
      <title>libhpxml &ndash; A High Performance XML Stream Parser</title>
      <style type="text/css">
         p { text-align:justify; }
      </style>
   </head>
   <body>
      <h1>libhpxml &ndash; A High Performance XML Stream Parser</h1>
      <h2>Download</h2>
      libhpxml currently is not a shared library. It is provided as a set
of
source files and can directly be compiled and linked into your project with
GCC.
      The current version is <a href="download/">available here</a>.
      <h2>Description</h2>
      <p>
      libhpxml is a high performance XML stream parser library written in C with a simple API. It
is intended to parse
      XML files very speed efficient. This may be required when processing
huge XML files
      like the <a href="http://wiki.openstreetmap.org/wiki/Planet.osm">OSM
planet file</a> which currently has 260GB+.
      <br>
      The development goals are <span style="font-weight:bold">speed
efficiency</span> and
 <span style="font-weight:bold">simple memory handling</span> to reduce the 
risk
of
      memory leaks.
      These objectives are achieved through
      </p>

      <ul>
         <li>avoidance of system calls (such as <span
style="font-family:monospace">malloc()</span>) as much as possible,</li>
         <li>usage of (nearly) static memory buffers, and</li>
         <li>avoidance of copying memory.</li>
      </ul>

      <p>
      Being a stream parser, libhpxml returns in a loop one XML element after
the other. It uses a
      result buffer which is initialized once and then reused for every 
element.
Thus, repeated calls to <span style="font-family:monospace">malloc()</span> and 
<span
style="font-family:monospace">free()</span> are omitted. The input data is read
      in blocks. The result buffer does not contain the data itself but just
pointers to the XML elements
      within the input buffer. Thus, data is not copied, it is just pointed to.
      </p>

      <h2>Usage</h2>

      <h3>Initialization</h3>
      <p>
      libhpxml provides a set of functions and structures. <span
         style="font-family:monospace">hpx_ctrl_t</span> is a control structure
which
         contains all relevant information for a XML stream. The contents of
         the structure are used internally by the library and should not be
         modified in any way. The structure is initialized with a call to
         <span style="font-family:monospace">hpx_init()</span> and must be 
freed
again
         with <span style="font-family:monospace">hpx_free()</span>.
Note that <span style="font-family:monospace">hpx_free()</span> will not close the file descriptor.
         </p>
         <pre>
   hpx_ctrl_t *hpx_init(int fd, int len);
   void hpx_free(hpx_ctrl_t *ctl);
</pre>
         <p>
         The arguments to <span style="font-family:monospace">hpx_init()</span>
is a file descriptor to an open XML file and the length
            of the block buffer. It will initialize a <span
style="font-family:monospace">hpx_ctrl_t</span> structure and returns a pointer
to it. In case of error <span style="font-family:monospace">NULL</span> 
is returned and <span style="font-family:monospace">errno</span> is set appropriately.
The buffer size must be at least as large as the longest XML element
         in the file but it is recommended to be much larger. The larger the 
buffer
the lesser
         the number of reads. If there is enough system memory available, it is
safe to choose 100MB or even more.
         </p>
<h4 id="mmap">Memory Mapping</h4>

<p>
Libhpxml now supports memory mapping through the system call
<span style="font-family:monospace">mmap()</span>. This is activated if 
<span style="font-family:monospace">hpx_init()</span> is called with a negative
<span style="font-face:underline;">len</span> parameter. In case of memory mapping, len must
be as long as the (negative value) of the total length of the XML input file.
Memory mapping of files greater than 2 GB is currently just supported on 64 bit architectures
(see manpage <span style="font-family:monospace">mmap(2)</span> or POSIX manpage
<span style="font-family:monospace">mmap(3)</span>, respectively).
</p>
<p>
The main application for memory mapping is if libhpxml is not just used as stream parser but
XML objects are kept in memory during the whole runtime. This is necessary if on-the-fly object
processing is not possible. This typically is the case if XML objects are nested or they depend
on each other. An example is the rendering process of OSM data.
<br>
Keeping pointers valid (see <a href="#hpx_get_elem"><span style="font-family:monospace">hpx_get_elem()</span></a>)
is still possible without memory mapping, but it requires that the buffer
is as large as the file itself because it has to pull in the whole file at once. Thus, this
works just if the system has enough memory. Memory mapping in contrast does not require physical memory,
hence, even a file with several hundred GB may be used.
<br>
Note that the preprocessor macro <span style="font-family:monospace">WITH_MMAP</span>
must be defined at compile time to compile libhpxml with <span style="font-family:monospace">mmap()</span> 
support.
If it was not compiled with <span style="font-family:monospace;">WITH_MMAP</span>, <span style="font-family:monospace">hpx_init()</span> 
will fail, in which case <span style="font-family:monospace;">NULL</span> is returned
and <span style="font-family:monospace;">errno</span> is set to
<span style="font-family:monospace;">EINVAL</span>.
</p>

         <h3>Supporting Functions</h3>
         <p>
         While parsing an XML file libhpxml returns pointers to the elements 
and
attributes.
         C strings are usually '\0'-terminated but this is not applicable here
because it would
         require that '\0' characters are inserted after each element, 
resulting
in huge data
         movement. Thus, libhpxml uses "B strings" which are hold in the <span
style="font-family:monospace">bstring_t</span> structure. The structure
contains a pointer to the string and its length. Additionally, a
         set of function is provided to handle those strings.
         </p>
   <pre>
   typedef struct bstring
   {
      int len;
      char *buf;
   } bstring_t;
   </pre>

         <h3>Processing Elements</h3>
         <p>
         After initializing the control structure, XML elements are 
subsequently
retrieved by repeated calls to <a id="hpx_get_elem"><span 
style="font-family:monospace">hpx_get_elem()</span></a>.
         </p>
         <pre>
   int hpx_get_elem(hpx_ctrl_t *ctl, bstring_t *b, int *in_tag, size_t *lno);
</pre>
         <p>
         The function processes the buffer and fills out the bstring pointing 
to
         the next XML element. <span style="font-family:monospace">ctl</span> 
is
the pointer to control structure. <span
style="font-family:monospace">in_tag</span>
         is filled with either 0 or 1, either if the XML element is a tag
         (&lt;...&gt;) or if it is literal text between tags. <span
style="font-family:monospace">lno</span> is filled
         with the line number at which this element starts. Both, <span
style="font-family:monospace">in_tag</span> and <span
style="font-family:monospace">lno</span> may be <span
style="font-family:monospace">NULL</span> if
         it is not used. <span
style="font-family:monospace">hpx_get_elem()</span> returns the length of the
bstring, 0 on EOF, and -1 in case of error.
         Such an element can now be parsed with a call to <span
style="font-family:monospace">hpx_process_elem()</span>.
         </p>
         <pre>
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

</pre>
         <p>
         It takes a bstring which contains an XML element and parses it into
         the <span style="font-family:monospace">hpx_tag_t</span> structure.
This structure may be initialized using
            <span style="font-family:monospace">hpx_tm_create()</span> but it
may also be initialized manually. In the latter
            case the structure member <span
style="font-family:monospace">mattr</span> must contain the size of the
attribute
            array. Otherwise the program may segfault. The argument to <span
               style="font-family:monospace">hpx_tm_create()</span> is the
            maximum number of expected attributes. The tag structure should be
            freed again with <span
style="font-family:monospace">hpx_tm_free()</span> after use. It is recommended
to
            reuse the tag structure. This reduces unnecessary memory management
            system calls.
            <br>
            Please note that a call to <span
style="font-family:monospace">hpx_get_elem()</span> may invalidate the
            pointers within previously filled-out tag structures because it might read in the
            next block of the input file. Thus, the tag must be processed
            <span style="font-weight:bold;">before</span> the next call to <span
style="font-family:monospace">hpx_get_elem()</span>.
            <br>
            The <span style="font-family:monospace">type</span> member of <span
style="font-family:monospace">hpx_tag_t</span>
            defines the type this XML element.
            Currently, the following types are known.
            <table>
               <tr><td>enum</td><td>Description</td><td>Example</td></tr>
               <tr><td style="font-family:monospace">HPX_ILL</td><td>Element
unknown. This may indicate a syntax error.</td></tr>
               <tr><td style="font-family:monospace">HPX_OPEN</td><td>An XML
opening tag.</td><td style="font-family:monospace">&lt;tagname
attrname="attrval"...&gt;</td></tr>
               <tr><td style="font-family:monospace">HPX_SINGLE</td><td>A
single, closed XML tag.</td><td style="font-family:monospace">&lt;tagname
attrname="attrval".../&gt;</td></tr>
               <tr><td style="font-family:monospace">HPX_CLOSE</td><td>An XML
closing tag.</td><td style="font-family:monospace">&lt;/tagname&gt;</td></tr>
               <tr><td style="font-family:monospace">HPX_LITERAL</td><td>No 
tag,
just text between tags.</td><td style="font-family:monospace"></td></tr>
               <tr><td
style="font-family:monospace">HPX_ATT</td><td>Declarations.</td><td
style="font-family:monospace">&lt;! ..... &gt;</td></tr>
               <tr><td
style="font-family:monospace">HPX_INSTR</td><td>Instructions.</td><td
style="font-family:monospace">&lt;? .... ?&gt;</td></tr>
               <tr><td
style="font-family:monospace">HPX_COMMENT</td><td>Comments.</td><td
style="font-family:monospace">&lt;!-- .... --&gt;</td></tr>
            </table>

         </p>
<pre>
   hpx_tag_t *hpx_tm_create(int n);
   void hpx_tm_free(hpx_tag_t *t);
</pre>

<p>
The tag structure further contains an array of attributes. The member <span
style="font-family:monospace">nattr</span>
contains the actual number of attributes parsed. It is always at most <span
style="font-family:monospace">mattr</span>
elements. If an XML tag has more than <span
style="font-family:monospace">mattr</span> elements they are just ignored. At
the current version there's no feedback to the calling function. This will be
improved in future releases.
The attributes themselves are stored each in an <span
style="font-family:monospace">hpx_attr_t</span> structure. It contains two
bstrings, one for the name and one for the value of the attribute. The third
member <span style="font-family:monospace">delim</span> keeps the delimiter of
the value which is either '\'' (single quote, 0x27) or '"' (double quote, 
0x22).
</p>

         <h2>Example</h2>

      <p>
      This example parses an XML file and outputs some stats about each XML
element.
      You can download the example <a href="example.c">directly here</a>.
      </p>

<?php


$gs = "geshi/geshi.php";
$s = file_get_contents("example.c");

if (file_exists($gs))
{
   require_once("geshi/geshi.php");
   $h = new GeSHi($s, "C");
   echo $h->parse_code();
}
else
{
   echo "<pre>$s</pre>";
}

?>

      <h2>Bugs and Caveats</h2>
      <p>
      libhpxml does not validate the XML file using e.g. DTD. Thus, it does not
      care about semantic errors. Syntactical ones of course are reported.
      In the current version, libhpxml is not thread-safe. The interface to the 
functions may change because it is in early development. The array of 
attributes within
      the <span style="font-family:monospace">hpx_tag_t</span> structure has a
static size and is not resized if an XML tag has more attributes
      as array entries are available. Currently, <span
style="font-family:monospace">hpx_process_elem()</span>
      does not report if the number of attributes would exceed the array (of 
course, it does not exhaust it).


      </p>

      <h2>Author</h2>
      <p>
      libhpxml is developed and maintained by <a
href="mailto:bf@abenteuerand.at">Bernhard R. Fischer, 2048R/5C5FFD47
&lt;bf@abenteuerland.at&gt;</a>.
<br>
Latest update 2012/01/09.
      </p>

      <h2>License</h2>
      <p>
      libhpxml is released under GNU GPLv3.
      </p>

   </body>
</html>

