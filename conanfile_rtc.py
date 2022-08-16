from conans import ConanFile


class glfwConan(ConanFile):
    name = "glfw"
    version = "3.3.8"
    url = "https://github.com/Esri/glfw"
    license = "https://github.com/Esri/glfw/LICENSE.md"
    description = (
        "GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan application development."
    )

    # RTC specific triple
    settings = "platform_architecture_target"

    def package(self):
        base = self.source_folder + "/"
        relative = "3rdparty/glfw/"

        # headers
        self.copy("*.h*", src=base + "include/GLFW/", dst=relative + "include/GLFW/")

        # libraries
        output = "output/" + str(self.settings.platform_architecture_target) + "/bin"
        self.copy("*" + self.name + "*", src=base + "../../" + output, dst=output)
