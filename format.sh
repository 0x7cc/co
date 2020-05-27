find include/ -name '*.h' -exec clang-format -i {} \;
find src/ -name '*.c' -exec clang-format -i {} \;
find tests/ -name '*.c' -exec clang-format -i {} \;
