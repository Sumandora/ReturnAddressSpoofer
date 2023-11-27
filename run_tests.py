import subprocess

project_dir = '.'
build_dir = 'tests'

compilers = [['gcc', 'g++'], ['clang', 'clang++']]
bits = [32, 64]
opt_levels = ['O0', 'O1', 'O2', 'O3', 'Ofast']

build_results = {}

for [c_compiler, cxx_compiler] in compilers:
    for bit in bits:
        for opt_level in opt_levels:
            build_name = f'{c_compiler}-{bit}-{opt_level}'

            c_args = f"-{opt_level} -m{bit}"
            cxx_args = f"-{opt_level} -m{bit}"

            if cxx_compiler == "clang++":
                cxx_args += " -fexperimental-library"

            cmake_flags = f'-DCMAKE_C_FLAGS="{c_args}" -DCMAKE_CXX_FLAGS="{cxx_args}"'

            configure_cmd = (f'CC={c_compiler} CXX={cxx_compiler} cmake -B"{build_dir}/{build_name}" {cmake_flags} '
                             f'"{project_dir}"')
            subprocess.run(configure_cmd, shell=True, check=True)

            build_cmd = f'cmake --build "{build_dir}/{build_name}"'
            subprocess.run(build_cmd, shell=True, check=True)

            exe = f'{build_dir}/{build_name}/Example/ReturnAddressSpooferExample'
            result = subprocess.run(exe, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)

            passed = result.stdout == "String length was: 13\n" and result.stderr == "" and result.returncode == 0
            build_results[build_name] = passed

print("Build results:")
for build_name, passed in sorted(build_results.items()):
    status = "Passed" if passed else "Failed"
    print(f"{build_name}: {status}")
