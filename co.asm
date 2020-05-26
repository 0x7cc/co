bits 64

global co_exec
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


co_get_rip:
  mov rax, [rsp]  ; 取出返回地址
  ret

co_exec:

  push rbp
  mov rbp, rsp

  mov r10, argv0

  mov rax , qword [r10 + co_context.rax]
  mov rbx , qword [r10 + co_context.rbx]
  mov rcx , qword [r10 + co_context.rcx]
  mov rdx , qword [r10 + co_context.rdx]
  mov rsi , qword [r10 + co_context.rsi]
  mov rdi , qword [r10 + co_context.rdi]
  mov rbp , qword [r10 + co_context.rbp]
  mov rsp , qword [r10 + co_context.rsp]
  mov r8  , qword [r10 + co_context.r8 ]
  mov r9  , qword [r10 + co_context.r9 ]
  mov r12 , qword [r10 + co_context.r12]
  mov r13 , qword [r10 + co_context.r13]
  mov r14 , qword [r10 + co_context.r14]
  mov r15 , qword [r10 + co_context.r15]

  ;;;;;;;;; 进入协程态

  call qword [r10 + co_context.rip]



  pop rbp

  ret

co_swap_context:
  mov r10, argv0
  mov r11, argv1

  mov qword [r10 + co_context.rax], rax ;
  mov qword [r10 + co_context.rbx], rbx ;
  mov qword [r10 + co_context.rcx], rcx ;
  mov qword [r10 + co_context.rdx], rdx ;
  mov qword [r10 + co_context.rsi], rsi ;
  mov qword [r10 + co_context.rdi], rdi ;
  mov qword [r10 + co_context.rbp], rbp ;
  mov qword [r10 + co_context.rsp], rsp ;
  mov qword [r10 + co_context.r8 ], r8  ;
  mov qword [r10 + co_context.r9 ], r9  ;
  mov qword [r10 + co_context.r12], r12 ;
  mov qword [r10 + co_context.r13], r13 ;
  mov qword [r10 + co_context.r14], r14 ;
  mov qword [r10 + co_context.r15], r15 ;
  mov qword [r10 + co_context.rip], .end ;

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
  mov r12 , qword [r11 + co_context.r12]
  mov r13 , qword [r11 + co_context.r13]
  mov r14 , qword [r11 + co_context.r14]
  mov r15 , qword [r11 + co_context.r15]
  jmp [r11 + co_context.rip]

  .end:
  ret

