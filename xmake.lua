set_project("MoCaChem")
set_version("0.0")
add_rules("mode.debug", "mode.release")
add_requires("tomlc99")

target("MoCaChem")
	set_kind("binary")
	add_files("src/*.c")
	add_includedirs("src/include")
	add_packages("tomlc99")
	if is_mode("debug") then
		set_symbols("debug")
		set_optimize("none")
		add_cflags("-Wall", "-Wextra")
	elseif is_mode("release") then
		set_optimize("fastest")
	end
