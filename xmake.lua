includes("lib/commonlibf4")

set_project("SafeDamageThreshold")
set_version("0.1.0")
set_license("MIT")
set_languages("c++23")
set_warnings("allextra")

add_rules("mode.debug", "mode.releasedbg")
add_requires("minhook v1.3.4")

target("SafeDamageThreshold")
    add_rules("commonlibf4.plugin", {
        name = "SafeDamageThreshold",
        author = "moshia mam / OpenAI",
        description = "Armor-derived damage threshold with safe zero clamping for Fallout 4 1.11.221"
    })
    add_packages("minhook")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

