import subprocess
import os

project_dir = '.'
build_dir = 'tests'

compilers = [['gcc', 'g++'], ['clang', 'clang++']]
bits = [32, 64]
opt_levels = ['O0', 'O1', 'O2', 'O3', 'Ofast', 'Os']

build_results = {}

for [cc, cxx] in compilers:
    for bit in bits:
        for opt_level in opt_levels:
            build_name = f'{cc}-{bit}-{opt_level}'

            c_args = f"-{opt_level} -m{bit}"
            cxx_args = f"-{opt_level} -m{bit}"

            if cxx == "clang++":
                cxx_args += " -fexperimental-library"

            cmake_flags = f'-DCMAKE_C_FLAGS="{c_args}" -DCMAKE_CXX_FLAGS="{cxx_args}"'

            configure_cmd = f'CC={cc} CXX={cxx} cmake -B"{build_dir}/{build_name}" \
                            {cmake_flags} "{project_dir}"'
            configure_proc = subprocess.run(configure_cmd, shell=True, check=False)
            if configure_proc.returncode != 0:
                build_results[build_name] = False
                continue

            nproc = os.cpu_count()
            build_cmd = f'cmake --build "{build_dir}/{build_name}" -j{nproc}'
            build_proc = subprocess.run(build_cmd, shell=True, check=False)
            if build_proc.returncode != 0:
                build_results[build_name] = False
                continue

            exe = f'{build_dir}/{build_name}/Example/ReturnAddressSpooferExample'
            result = subprocess.run(exe, stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE, text=True, check=False)

            passed = result.stdout == "String length was: 13\n" \
                and result.stderr == "" and result.returncode == 0
            build_results[build_name] = passed

print("Build results:")
for build_name, passed in sorted(build_results.items()):
    status = "Passed" if passed else "Failed"
    print(f"{build_name}: {status}")

exit(sum([1 for passed in build_results.values() if not passed]))
