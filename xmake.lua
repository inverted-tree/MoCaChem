set_project("MoCaChem")
set_version("0.0")
add_rules("mode.debug", "mode.release")

target("MoCaChem")
	set_kind("binary")
	add_files("src/*.c")
	add_includedirs("src/include")
	if is_mode("debug") then
		set_symbols("debug")
		set_optimize("none")
	elseif is_mode("release") then
		set_optimize("fastest")
	end
