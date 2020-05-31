bits 64

globaL co_atomic_set
globaL co_atomic_inc
global co_atomic_dec
global co_atomic_add
global co_atomic_sub
global co_store_context
global co_load_context
global co_swap_context

%define elf64_fastcall_argv0 rdi
%define elf64_fastcall_argv1 rsi
%define elf64_fastcall_argv2 rdx
%define elf64_fastcall_argv3 rcx

%define win64_fastcall_argv0 rcx
%define win64_fastcall_argv1 rdx
%define win64_fastcall_argv2 r8
%define win64_fastcall_argv3 r9

%ifidn __OUTPUT_FORMAT__, elf64
  %define argv0 elf64_fastcall_argv0
  %define argv1 elf64_fastcall_argv1
  %define argv2 elf64_fastcall_argv2
  %define argv3 elf64_fastcall_argv3
%elifidn __OUTPUT_FORMAT__, macho64
  %define argv0 elf64_fastcall_argv0
  %define argv1 elf64_fastcall_argv1
  %define argv2 elf64_fastcall_argv2
  %define argv3 elf64_fastcall_argv3
%elifidn __OUTPUT_FORMAT__, win64
  %define argv0 win64_fastcall_argv0
  %define argv1 win64_fastcall_argv1
  %define argv2 win64_fastcall_argv2
  %define argv3 win64_fastcall_argv3
%else
  %error "目前只支持64位格式的fastcall"
%endif

struc co_context
.rax resb 8
.rbx resb 8
.rcx resb 8
.rdx resb 8
.rsi resb 8
.rdi resb 8
.rsp resb 8
.rbp resb 8
.rip resb 8
.r8  resb 8
.r9  resb 8
.r12 resb 8
.r13 resb 8
.r14 resb 8
.r15 resb 8
endstruc

section .text

co_atomic_set:
  xchg qword [argv0], argv1
  ret

co_atomic_inc:
  lock inc qword [argv0]
  ret

co_atomic_dec:
  lock dec qword [argv0]
  ret

co_atomic_add:
  lock add qword [argv0], argv1
  ret

co_atomic_sub:
  lock sub qword [argv0], argv1
  ret

co_store_context:
  mov r10, argv0

  mov qword [r10 + co_context.rax], rax
  mov qword [r10 + co_context.rbx], rbx
  mov qword [r10 + co_context.rcx], rcx
  mov qword [r10 + co_context.rdx], rdx
  mov qword [r10 + co_context.rsi], rsi
  mov qword [r10 + co_context.rdi], rdi
  mov qword [r10 + co_context.rbp], rbp
  mov qword [r10 + co_context.r8 ], r8
  mov qword [r10 + co_context.r9 ], r9
  ; NOTE: r10和r11寄存器不需要保存和恢复
  mov qword [r10 + co_context.r12], r12
  mov qword [r10 + co_context.r13], r13
  mov qword [r10 + co_context.r14], r14
  mov qword [r10 + co_context.r15], r15

  ; 把rip指向本函数返回的地址, 这里就需要计算一次函数返回后rsp的位置
  mov r11, rsp
  add r11, 8
  mov qword [r10 + co_context.rsp], r11

  ; 把rip指向本函数返回的地址
  mov r11, [rsp]
  mov qword [r10 + co_context.rip], r11

  ; r11寄存器并不需要清理
  ret

co_load_context:
  mov r11 , argv0
  mov rax , qword [r11 + co_context.rax]
  mov rbx , qword [r11 + co_context.rbx]
  mov rcx , qword [r11 + co_context.rcx]
  mov rdx , qword [r11 + co_context.rdx]
  mov rsi , qword [r11 + co_context.rsi]
  mov rdi , qword [r11 + co_context.rdi]
  mov rbp , qword [r11 + co_context.rbp]
  mov rsp , qword [r11 + co_context.rsp]
  mov r8  , qword [r11 + co_context.r8 ]
  mov r9  , qword [r11 + co_context.r9 ]
  ; NOTE: r10和r11寄存器不需要保存和恢复
  mov r12 , qword [r11 + co_context.r12]
  mov r13 , qword [r11 + co_context.r13]
  mov r14 , qword [r11 + co_context.r14]
  mov r15 , qword [r11 + co_context.r15]
  jmp qword [r11 + co_context.rip]

  ret

co_swap_context:
  mov r10, argv0

  mov qword [r10 + co_context.rax], rax
  mov qword [r10 + co_context.rbx], rbx
  mov qword [r10 + co_context.rcx], rcx
  mov qword [r10 + co_context.rdx], rdx
  mov qword [r10 + co_context.rsi], rsi
  mov qword [r10 + co_context.rdi], rdi
  mov qword [r10 + co_context.rbp], rbp
  mov qword [r10 + co_context.rsp], rsp
  mov qword [r10 + co_context.r8 ], r8
  mov qword [r10 + co_context.r9 ], r9
  ; NOTE: r10和r11寄存器不需要保存和恢复
  mov qword [r10 + co_context.r12], r12
  mov qword [r10 + co_context.r13], r13
  mov qword [r10 + co_context.r14], r14
  mov qword [r10 + co_context.r15], r15

  ; 把rip指向本函数返回的地址, 这里就需要计算一次函数返回后rsp的位置
  mov r11, rsp
  add r11, 8
  mov qword [r10 + co_context.rsp], r11

  ; 把rip指向本函数返回的地址
  mov r11, [rsp]
  mov qword [r10 + co_context.rip], r11

  mov r11, argv1
  mov rax , qword [r11 + co_context.rax]
  mov rbx , qword [r11 + co_context.rbx]
  mov rcx , qword [r11 + co_context.rcx]
  mov rdx , qword [r11 + co_context.rdx]
  mov rsi , qword [r11 + co_context.rsi]
  mov rdi , qword [r11 + co_context.rdi]
  mov rbp , qword [r11 + co_context.rbp]
  mov rsp , qword [r11 + co_context.rsp]
  mov r8  , qword [r11 + co_context.r8 ]
  mov r9  , qword [r11 + co_context.r9 ]
  ; NOTE: r10和r11寄存器不需要保存和恢复
  mov r12 , qword [r11 + co_context.r12]
  mov r13 , qword [r11 + co_context.r13]
  mov r14 , qword [r11 + co_context.r14]
  mov r15 , qword [r11 + co_context.r15]
  jmp qword [r11 + co_context.rip]

  ret

