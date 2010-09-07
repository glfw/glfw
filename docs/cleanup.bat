@echo off

REM ----------------------------------------------------------------------
REM Windows cleanup batch file for the GLFW documentation.
REM ----------------------------------------------------------------------

REM GLFW Reference Manual
if exist glfwrm.dvi del glfwrm.dvi
if exist glfwrm.aux del glfwrm.aux
if exist glfwrm.log del glfwrm.log
if exist glfwrm.out del glfwrm.out
if exist glfwrm.pdf del glfwrm.pdf
if exist glfwrm.toc del glfwrm.toc
if exist glfwrm.lot del glfwrm.lot

REM GLFW Users Guide
if exist glfwug.dvi del glfwug.dvi
if exist glfwug.aux del glfwug.aux
if exist glfwug.log del glfwug.log
if exist glfwug.out del glfwug.out
if exist glfwug.pdf del glfwug.pdf
if exist glfwug.toc del glfwug.toc
