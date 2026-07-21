add_rules("mode.debug", "mode.release")

add_requires("spdlog")
set_languages("c++23")

target("scrapcheese")
    set_kind("shared")
    add_files("src/*.cpp")
    add_packages("spdlog")

target("scrapcheese_injector")
    set_kind("binary")
    add_files("src_injector/*.cpp")
    add_syslinks("user32", "psapi", "shell32")