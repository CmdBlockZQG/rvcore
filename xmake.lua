set_languages("cxx17")
set_warnings("all", "error")
set_optimize("fastest")

set_targetdir("build")
add_includedirs("include")

-- 公共动态库
target("common")
  set_kind("shared")
  set_symbols("hidden")
  add_files("common/**.cc")

-- ISA模拟器
target("core-emu")
  -- 依赖公共动态库
  add_deps("common")
  -- 依赖支持环境，和sptenv一起编译
  add_includedirs("sptenv/include")
  add_files("sptenv/**.cc")
  -- 生成动态库文件，供testbench运行时加载
  set_kind("shared")
  set_symbols("hidden")
  add_files("core-emu/**.cc")

-- 简单流水线实现
target("core-rtl-a")
  -- 依赖公共动态库
  add_deps("common")
  -- 依赖支持环境，和sptenv一起编译
  add_includedirs("sptenv/include")
  add_files("sptenv/**.cc")
  -- 依赖verilated模型
  add_includedirs(
    "core-rtl-a/verilated_obj",
    "/usr/local/share/verilator/include",
    "/usr/local/share/verilator/include/vltstd"
  )
  add_linkdirs("core-rtl-a/verilated_obj")
  add_links("verilated", "Vtop")
  -- 生成动态库文件，供testbench运行时加载
  set_kind("shared")
  set_symbols("hidden")
  add_files("core-rtl-a/*.cc")

-- testbench
target("testbench")
  add_deps("common")
  add_links("readline")
  set_kind("binary")
  add_files("testbench/**.cc")
