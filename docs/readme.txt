Introduction
------------

The GLFW documentation is written in LaTeX.  Besides being powerful, LaTeX is
also very attractive since all the necessary tools for dealing with LaTeX
documentation are both free and ported to a wide variety of platforms. Another
advantage is that the LaTeX files are written in plain text, which means that
version control systems such as CVS handle them perfectly without having to
treat the documents as binary files.


The documents
-------------

There are two main documents:

 glfwrm.tex - The GLFW Reference Manual
 glfwug.tex - The GLFW Users Guide

In addition, there is a common LaTeX style file that sets up things
such as page formatting and useful macros:

 glfwdoc.sty - Common GLFW document styles and macros


Requirements
------------

Of course you need LaTeX installed on your system in order to compile the GLFW
documentation.  If you are using a Unix-like operating system, then your
package system most likely has a version of LaTeX adapted for your system.  If
not, the easiest way to get a full LaTeX system is to download/get the TeXLive
CD from http://www.tug.org/texlive/.  It has all the necessary software for
Windows, Mac OS X and most popular Unix-like operating systems.

A number of LaTeX packages have to be installed in order to compile the
GLFW documentation successfully:

 color
 fancyhdr
 hyperref
 lastpage
 listings
 needspace
 textcase
 times
 titling

These packages are all available on the TeXLive CD. Just make sure that
you have checked all these packages when installing TeXLive, or get them
in some other way if you do not have the TeXLive CD.

