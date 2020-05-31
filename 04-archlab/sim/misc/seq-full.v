assign icode = 
    (imem_error ? INOP : imem_icode);

assign ifun = 
    (imem_error ? FNONE : imem_ifun);

assign instr_valid = 
    (icode == INOP | icode == IHALT | icode == IRRMOVQ | icode == IIRMOVQ
       | icode == IRMMOVQ | icode == IMRMOVQ | icode == IOPQ | icode == 
      IJXX | icode == ICALL | icode == IRET | icode == IPUSHQ | icode == 
      IPOPQ);

assign need_regids = 
    (icode == IRRMOVQ | icode == IOPQ | icode == IPUSHQ | icode == IPOPQ | 
      icode == IIRMOVQ | icode == IRMMOVQ | icode == IMRMOVQ);

assign need_valC = 
    (icode == IIRMOVQ | icode == IRMMOVQ | icode == IMRMOVQ | icode == IJXX
       | icode == ICALL);

assign srcA = 
    ((icode == IRRMOVQ | icode == IRMMOVQ | icode == IOPQ | icode == IPUSHQ
        ) ? rA : (icode == IPOPQ | icode == IRET) ? RRSP : RNONE);

assign srcB = 
    ((icode == IOPQ | icode == IRMMOVQ | icode == IMRMOVQ) ? rB : (icode
         == IPUSHQ | icode == IPOPQ | icode == ICALL | icode == IRET) ? 
      RRSP : RNONE);

assign dstE = 
    (((icode == IRRMOVQ) & Cnd) ? rB : (icode == IIRMOVQ | icode == IOPQ)
       ? rB : (icode == IPUSHQ | icode == IPOPQ | icode == ICALL | icode
         == IRET) ? RRSP : RNONE);

assign dstM = 
    ((icode == IMRMOVQ | icode == IPOPQ) ? rA : RNONE);

assign aluA = 
    ((icode == IRRMOVQ | icode == IOPQ) ? valA : (icode == IIRMOVQ | icode
         == IRMMOVQ | icode == IMRMOVQ) ? valC : (icode == ICALL | icode
         == IPUSHQ) ? -8 : (icode == IRET | icode == IPOPQ) ? 8 : 0);

assign aluB = 
    ((icode == IRMMOVQ | icode == IMRMOVQ | icode == IOPQ | icode == ICALL
         | icode == IPUSHQ | icode == IRET | icode == IPOPQ) ? valB : (
        icode == IRRMOVQ | icode == IIRMOVQ) ? 0 : 0);

assign alufun = 
    ((icode == IOPQ) ? ifun : ALUADD);

assign set_cc = 
    (icode == IOPQ);

assign mem_read = 
    (icode == IMRMOVQ | icode == IPOPQ | icode == IRET);

assign mem_write = 
    (icode == IRMMOVQ | icode == IPUSHQ | icode == ICALL);

assign mem_addr = 
    ((icode == IRMMOVQ | icode == IPUSHQ | icode == ICALL | icode == 
        IMRMOVQ) ? valE : (icode == IPOPQ | icode == IRET) ? valA : 0);

assign mem_data = 
    ((icode == IRMMOVQ | icode == IPUSHQ) ? valA : (icode == ICALL) ? valP
       : 0);

assign Stat = 
    ((imem_error | dmem_error) ? SADR : ~instr_valid ? SINS : (icode == 
        IHALT) ? SHLT : SAOK);

assign new_pc = 
    ((icode == ICALL) ? valC : ((icode == IJXX) & Cnd) ? valC : (icode == 
        IRET) ? valM : valP);

