
kernels.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <naive_rotate>:
   0:	85 ff                	test   %edi,%edi
   2:	7e 7a                	jle    7e <naive_rotate+0x7e>
   4:	8d 4f ff             	lea    -0x1(%rdi),%ecx
   7:	48 63 c7             	movslq %edi,%rax
   a:	53                   	push   %rbx
   b:	4c 8d 14 40          	lea    (%rax,%rax,2),%r10
   f:	89 c8                	mov    %ecx,%eax
  11:	0f af c7             	imul   %edi,%eax
  14:	f7 df                	neg    %edi
  16:	4d 01 d2             	add    %r10,%r10
  19:	48 63 ff             	movslq %edi,%rdi
  1c:	48 98                	cltq   
  1e:	4c 8d 04 40          	lea    (%rax,%rax,2),%r8
  22:	48 01 c8             	add    %rcx,%rax
  25:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
  29:	4e 8d 0c 42          	lea    (%rdx,%r8,2),%r9
  2d:	4c 8d 04 7f          	lea    (%rdi,%rdi,2),%r8
  31:	48 8d 5c 42 06       	lea    0x6(%rdx,%rax,2),%rbx
  36:	48 8d 04 49          	lea    (%rcx,%rcx,2),%rax
  3a:	4d 01 c0             	add    %r8,%r8
  3d:	4c 8d 5c 00 06       	lea    0x6(%rax,%rax,1),%r11
  42:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
  48:	49 8d 3c 33          	lea    (%r11,%rsi,1),%rdi
  4c:	4c 89 ca             	mov    %r9,%rdx
  4f:	48 89 f0             	mov    %rsi,%rax
  52:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
  58:	8b 08                	mov    (%rax),%ecx
  5a:	48 83 c0 06          	add    $0x6,%rax
  5e:	89 0a                	mov    %ecx,(%rdx)
  60:	0f b7 48 fe          	movzwl -0x2(%rax),%ecx
  64:	66 89 4a 04          	mov    %cx,0x4(%rdx)
  68:	4c 01 c2             	add    %r8,%rdx
  6b:	48 39 f8             	cmp    %rdi,%rax
  6e:	75 e8                	jne    58 <naive_rotate+0x58>
  70:	49 83 c1 06          	add    $0x6,%r9
  74:	4c 01 d6             	add    %r10,%rsi
  77:	49 39 d9             	cmp    %rbx,%r9
  7a:	75 cc                	jne    48 <naive_rotate+0x48>
  7c:	5b                   	pop    %rbx
  7d:	c3                   	retq   
  7e:	f3 c3                	repz retq 

0000000000000080 <rotate>:
  80:	85 ff                	test   %edi,%edi
  82:	0f 8e c5 00 00 00    	jle    14d <rotate+0xcd>
  88:	44 8d 4f ff          	lea    -0x1(%rdi),%r9d
  8c:	41 56                	push   %r14
  8e:	41 55                	push   %r13
  90:	41 54                	push   %r12
  92:	55                   	push   %rbp
  93:	41 89 fa             	mov    %edi,%r10d
  96:	53                   	push   %rbx
  97:	89 fb                	mov    %edi,%ebx
  99:	41 f7 da             	neg    %r10d
  9c:	41 0f af d9          	imul   %r9d,%ebx
  a0:	41 83 e1 fe          	and    $0xfffffffe,%r9d
  a4:	45 01 d2             	add    %r10d,%r10d
  a7:	45 31 db             	xor    %r11d,%r11d
  aa:	41 83 c1 02          	add    $0x2,%r9d
  ae:	44 8d 24 1f          	lea    (%rdi,%rbx,1),%r12d
  b2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
  b8:	41 8d 6b 01          	lea    0x1(%r11),%ebp
  bc:	89 d9                	mov    %ebx,%ecx
  be:	31 c0                	xor    %eax,%eax
  c0:	44 8d 40 01          	lea    0x1(%rax),%r8d
  c4:	44 39 c7             	cmp    %r8d,%edi
  c7:	7e 32                	jle    fb <rotate+0x7b>
  c9:	41 89 c8             	mov    %ecx,%r8d
  cc:	41 29 f8             	sub    %edi,%r8d
  cf:	4d 63 c0             	movslq %r8d,%r8
  d2:	4f 8d 04 40          	lea    (%r8,%r8,2),%r8
  d6:	4e 8d 2c 42          	lea    (%rdx,%r8,2),%r13
  da:	44 8d 44 05 00       	lea    0x0(%rbp,%rax,1),%r8d
  df:	4d 63 c0             	movslq %r8d,%r8
  e2:	4f 8d 04 40          	lea    (%r8,%r8,2),%r8
  e6:	4e 8d 04 46          	lea    (%rsi,%r8,2),%r8
  ea:	45 8b 30             	mov    (%r8),%r14d
  ed:	45 89 75 00          	mov    %r14d,0x0(%r13)
  f1:	45 0f b7 40 04       	movzwl 0x4(%r8),%r8d
  f6:	66 45 89 45 04       	mov    %r8w,0x4(%r13)
  fb:	39 c7                	cmp    %eax,%edi
  fd:	7e 2b                	jle    12a <rotate+0xaa>
  ff:	4c 63 c1             	movslq %ecx,%r8
 102:	4f 8d 04 40          	lea    (%r8,%r8,2),%r8
 106:	4e 8d 2c 42          	lea    (%rdx,%r8,2),%r13
 10a:	46 8d 04 18          	lea    (%rax,%r11,1),%r8d
 10e:	4d 63 c0             	movslq %r8d,%r8
 111:	4f 8d 04 40          	lea    (%r8,%r8,2),%r8
 115:	4e 8d 04 46          	lea    (%rsi,%r8,2),%r8
 119:	45 8b 30             	mov    (%r8),%r14d
 11c:	45 89 75 00          	mov    %r14d,0x0(%r13)
 120:	45 0f b7 40 04       	movzwl 0x4(%r8),%r8d
 125:	66 45 89 45 04       	mov    %r8w,0x4(%r13)
 12a:	83 c0 02             	add    $0x2,%eax
 12d:	44 01 d1             	add    %r10d,%ecx
 130:	44 39 c8             	cmp    %r9d,%eax
 133:	75 8b                	jne    c0 <rotate+0x40>
 135:	83 c3 01             	add    $0x1,%ebx
 138:	41 01 fb             	add    %edi,%r11d
 13b:	44 39 e3             	cmp    %r12d,%ebx
 13e:	0f 85 74 ff ff ff    	jne    b8 <rotate+0x38>
 144:	5b                   	pop    %rbx
 145:	5d                   	pop    %rbp
 146:	41 5c                	pop    %r12
 148:	41 5d                	pop    %r13
 14a:	41 5e                	pop    %r14
 14c:	c3                   	retq   
 14d:	f3 c3                	repz retq 
 14f:	90                   	nop

0000000000000150 <naive_smooth>:
 150:	85 ff                	test   %edi,%edi
 152:	0f 8e 8a 01 00 00    	jle    2e2 <naive_smooth+0x192>
 158:	41 57                	push   %r15
 15a:	8d 47 ff             	lea    -0x1(%rdi),%eax
 15d:	41 56                	push   %r14
 15f:	41 55                	push   %r13
 161:	41 54                	push   %r12
 163:	41 89 fc             	mov    %edi,%r12d
 166:	55                   	push   %rbp
 167:	53                   	push   %rbx
 168:	89 44 24 e4          	mov    %eax,-0x1c(%rsp)
 16c:	48 63 c7             	movslq %edi,%rax
 16f:	48 89 54 24 f0       	mov    %rdx,-0x10(%rsp)
 174:	48 8d 04 40          	lea    (%rax,%rax,2),%rax
 178:	c7 44 24 ec 00 00 00 	movl   $0x0,-0x14(%rsp)
 17f:	00 
 180:	48 89 74 24 c8       	mov    %rsi,-0x38(%rsp)
 185:	48 01 c0             	add    %rax,%rax
 188:	48 89 44 24 f8       	mov    %rax,-0x8(%rsp)
 18d:	48 8d 46 06          	lea    0x6(%rsi),%rax
 191:	48 89 44 24 c0       	mov    %rax,-0x40(%rsp)
 196:	83 44 24 ec 01       	addl   $0x1,-0x14(%rsp)
 19b:	8b 74 24 e4          	mov    -0x1c(%rsp),%esi
 19f:	bf 00 00 00 00       	mov    $0x0,%edi
 1a4:	8b 5c 24 ec          	mov    -0x14(%rsp),%ebx
 1a8:	c7 44 24 d8 00 00 00 	movl   $0x0,-0x28(%rsp)
 1af:	00 
 1b0:	89 d8                	mov    %ebx,%eax
 1b2:	83 e8 02             	sub    $0x2,%eax
 1b5:	0f 49 f8             	cmovns %eax,%edi
 1b8:	39 de                	cmp    %ebx,%esi
 1ba:	89 d8                	mov    %ebx,%eax
 1bc:	0f 4e c6             	cmovle %esi,%eax
 1bf:	89 7c 24 dc          	mov    %edi,-0x24(%rsp)
 1c3:	48 8b 5c 24 f0       	mov    -0x10(%rsp),%rbx
 1c8:	41 0f af fc          	imul   %r12d,%edi
 1cc:	89 44 24 e0          	mov    %eax,-0x20(%rsp)
 1d0:	83 c0 01             	add    $0x1,%eax
 1d3:	89 44 24 bc          	mov    %eax,-0x44(%rsp)
 1d7:	48 89 5c 24 d0       	mov    %rbx,-0x30(%rsp)
 1dc:	89 7c 24 e8          	mov    %edi,-0x18(%rsp)
 1e0:	44 8b 4c 24 dc       	mov    -0x24(%rsp),%r9d
 1e5:	8b 44 24 e0          	mov    -0x20(%rsp),%eax
 1e9:	41 39 c1             	cmp    %eax,%r9d
 1ec:	0f 8f ee 00 00 00    	jg     2e0 <naive_smooth+0x190>
 1f2:	83 44 24 d8 01       	addl   $0x1,-0x28(%rsp)
 1f7:	8b 74 24 e4          	mov    -0x1c(%rsp),%esi
 1fb:	bf 00 00 00 00       	mov    $0x0,%edi
 200:	8b 44 24 d8          	mov    -0x28(%rsp),%eax
 204:	44 8b 54 24 e8       	mov    -0x18(%rsp),%r10d
 209:	89 c3                	mov    %eax,%ebx
 20b:	89 c5                	mov    %eax,%ebp
 20d:	83 eb 02             	sub    $0x2,%ebx
 210:	0f 48 df             	cmovs  %edi,%ebx
 213:	39 c6                	cmp    %eax,%esi
 215:	0f 4e ee             	cmovle %esi,%ebp
 218:	4c 63 fb             	movslq %ebx,%r15
 21b:	45 31 db             	xor    %r11d,%r11d
 21e:	41 89 ee             	mov    %ebp,%r14d
 221:	44 8d 6d 01          	lea    0x1(%rbp),%r13d
 225:	31 f6                	xor    %esi,%esi
 227:	41 29 de             	sub    %ebx,%r14d
 22a:	31 c0                	xor    %eax,%eax
 22c:	4d 01 fe             	add    %r15,%r14
 22f:	41 29 dd             	sub    %ebx,%r13d
 232:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
 238:	39 eb                	cmp    %ebp,%ebx
 23a:	7f 41                	jg     27d <naive_smooth+0x12d>
 23c:	49 63 ca             	movslq %r10d,%rcx
 23f:	4c 8b 44 24 c8       	mov    -0x38(%rsp),%r8
 244:	49 8d 14 0f          	lea    (%r15,%rcx,1),%rdx
 248:	4c 01 f1             	add    %r14,%rcx
 24b:	48 8d 0c 49          	lea    (%rcx,%rcx,2),%rcx
 24f:	48 8d 14 52          	lea    (%rdx,%rdx,2),%rdx
 253:	49 8d 14 50          	lea    (%r8,%rdx,2),%rdx
 257:	4c 8b 44 24 c0       	mov    -0x40(%rsp),%r8
 25c:	4d 8d 04 48          	lea    (%r8,%rcx,2),%r8
 260:	0f b7 0a             	movzwl (%rdx),%ecx
 263:	48 83 c2 06          	add    $0x6,%rdx
 267:	01 c8                	add    %ecx,%eax
 269:	0f b7 4a fc          	movzwl -0x4(%rdx),%ecx
 26d:	01 cf                	add    %ecx,%edi
 26f:	0f b7 4a fe          	movzwl -0x2(%rdx),%ecx
 273:	01 ce                	add    %ecx,%esi
 275:	4c 39 c2             	cmp    %r8,%rdx
 278:	75 e6                	jne    260 <naive_smooth+0x110>
 27a:	45 01 eb             	add    %r13d,%r11d
 27d:	41 83 c1 01          	add    $0x1,%r9d
 281:	45 01 e2             	add    %r12d,%r10d
 284:	44 39 4c 24 bc       	cmp    %r9d,-0x44(%rsp)
 289:	75 ad                	jne    238 <naive_smooth+0xe8>
 28b:	99                   	cltd   
 28c:	48 8b 5c 24 d0       	mov    -0x30(%rsp),%rbx
 291:	41 f7 fb             	idiv   %r11d
 294:	48 83 c3 06          	add    $0x6,%rbx
 298:	66 89 43 fa          	mov    %ax,-0x6(%rbx)
 29c:	89 f8                	mov    %edi,%eax
 29e:	99                   	cltd   
 29f:	41 f7 fb             	idiv   %r11d
 2a2:	66 89 43 fc          	mov    %ax,-0x4(%rbx)
 2a6:	89 f0                	mov    %esi,%eax
 2a8:	99                   	cltd   
 2a9:	41 f7 fb             	idiv   %r11d
 2ac:	66 89 43 fe          	mov    %ax,-0x2(%rbx)
 2b0:	44 3b 64 24 d8       	cmp    -0x28(%rsp),%r12d
 2b5:	48 89 5c 24 d0       	mov    %rbx,-0x30(%rsp)
 2ba:	0f 85 20 ff ff ff    	jne    1e0 <naive_smooth+0x90>
 2c0:	48 8b 5c 24 f8       	mov    -0x8(%rsp),%rbx
 2c5:	48 01 5c 24 f0       	add    %rbx,-0x10(%rsp)
 2ca:	44 39 64 24 ec       	cmp    %r12d,-0x14(%rsp)
 2cf:	0f 85 c1 fe ff ff    	jne    196 <naive_smooth+0x46>
 2d5:	5b                   	pop    %rbx
 2d6:	5d                   	pop    %rbp
 2d7:	41 5c                	pop    %r12
 2d9:	41 5d                	pop    %r13
 2db:	41 5e                	pop    %r14
 2dd:	41 5f                	pop    %r15
 2df:	c3                   	retq   
 2e0:	0f 0b                	ud2    
 2e2:	f3 c3                	repz retq 
 2e4:	66 90                	xchg   %ax,%ax
 2e6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
 2ed:	00 00 00 

00000000000002f0 <smooth>:
 2f0:	e9 5b fe ff ff       	jmpq   150 <naive_smooth>
 2f5:	90                   	nop
 2f6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
 2fd:	00 00 00 

0000000000000300 <register_rotate_functions>:
 300:	48 8d 35 00 00 00 00 	lea    0x0(%rip),%rsi        # 307 <register_rotate_functions+0x7>
 307:	48 8d 3d 00 00 00 00 	lea    0x0(%rip),%rdi        # 30e <register_rotate_functions+0xe>
 30e:	48 83 ec 08          	sub    $0x8,%rsp
 312:	e8 00 00 00 00       	callq  317 <register_rotate_functions+0x17>
 317:	48 8d 35 00 00 00 00 	lea    0x0(%rip),%rsi        # 31e <register_rotate_functions+0x1e>
 31e:	48 8d 3d 00 00 00 00 	lea    0x0(%rip),%rdi        # 325 <register_rotate_functions+0x25>
 325:	48 83 c4 08          	add    $0x8,%rsp
 329:	e9 00 00 00 00       	jmpq   32e <register_rotate_functions+0x2e>
 32e:	66 90                	xchg   %ax,%ax

0000000000000330 <register_smooth_functions>:
 330:	48 8d 35 00 00 00 00 	lea    0x0(%rip),%rsi        # 337 <register_smooth_functions+0x7>
 337:	48 8d 3d 00 00 00 00 	lea    0x0(%rip),%rdi        # 33e <register_smooth_functions+0xe>
 33e:	48 83 ec 08          	sub    $0x8,%rsp
 342:	e8 00 00 00 00       	callq  347 <register_smooth_functions+0x17>
 347:	48 8d 35 00 00 00 00 	lea    0x0(%rip),%rsi        # 34e <register_smooth_functions+0x1e>
 34e:	48 8d 3d 00 00 00 00 	lea    0x0(%rip),%rdi        # 355 <register_smooth_functions+0x25>
 355:	48 83 c4 08          	add    $0x8,%rsp
 359:	e9 00 00 00 00       	jmpq   35e <register_smooth_functions+0x2e>
