.data					# section declaration

        # Useful offset constants for accessing members of a 
        # struct mp1_blink_struct structure
        LOCATION   = 0    
        ON_CHAR    = 2
        OFF_CHAR   = 3 
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:
        
	movl    vmem_base_addr(,1),%edx
    movb    %cl,(%edx,%eax,2) 
	ret

# void mp1 rtc tasklet (unsigned long)
# Interface: C style fucntion
#    Inputs: an unsigned long value
#    Outputs: Walk through the mp1 list and control the 
#    toggle rates of each character with length field.
# Registers:    
#    %edi: stores the pointer to the linked list  
#    %ax : used as temporal register (storing lcoation, countdwon field)
#    %cx : used as temporal register (storing status field)
    
mp1_rtc_tasklet:
    #calling convention
    pushl %ebp
    movl %esp, %ebp
    #callee saved registers
    pushl %ebx
    pushl %esi
    pushl %edi

    movl mp1_list_head, %edi #store the pointer of list
    #walk through the linked list
    WALK_THROUGH:
        #Jump to the end if encountering a NULL pointer
        cmpl $0, %edi
        je   MP1_RTC_TASKLET_OVER

        #Decrement the value of count down and check if it's zero
        movw COUNTDOWN(%edi), %cx
        subw $1, %cx
        movw %cx, COUNTDOWN(%edi)
        #If the countdown = 0, then check status
        cmpw $0, %cx
        jne COUNTDOWN_NOT_ZERO_MOVE_ON
        #countdown != 0, move on
        movw STATUS(%edi), %cx
        cmpw $0, %cx
        je STATUS_ZERO
        cmpw $1, %cx
        je STATUS_ONE
        #status can only be 1 or 0, so program do not reach here
    COUNTDOWN_NOT_ZERO_MOVE_ON:
        #move to the next list element
        movl NEXT(%edi), %edi
        jmp  WALK_THROUGH

    STATUS_ZERO:
        #change the status to one
        movw $1, %cx
        movw %cx, STATUS(%edi)
        #prepare to call mp1_poke
        andl $0, %eax #clear %eax
        movzwl (%edi), %eax #store the location in %ax
        movb ON_CHAR(%edi), %cl
        pushl %edx
        call mp1_poke
        popl %edx
        #update countdown
        movw ON_LENGTH(%edi), %cx
        movw %cx, COUNTDOWN(%edi)
        jmp COUNTDOWN_NOT_ZERO_MOVE_ON

    STATUS_ONE:
        #change the status to zero
        movw $0, %cx
        movw %cx, STATUS(%edi)
        #prepare to call mp1_poke
        andl $0, %eax #clear %eax
        movzwl (%edi), %eax #store the location in %ax
        movb OFF_CHAR(%edi), %cl
        pushl %edx
        call mp1_poke
        popl %edx
        #update countdown
        movw OFF_LENGTH(%edi), %cx
        movw %cx, COUNTDOWN(%edi)
        jmp COUNTDOWN_NOT_ZERO_MOVE_ON



    MP1_RTC_TASKLET_OVER:
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret






# int mp1_ioctl (unsigned long arg, unsigned long cmd)
# Interface: C style fucntion with calling convention
# Inputs: %cl  - The byte you wish to write
#         %eax - Offset from the start of video memory that you wish
#                   to write to
# Outputs: Text-mode video screen is written to at location %eax with
#          the byte in %cl
# Registers: Clobbers EDX
mp1_ioctl:
    movl 8(%esp), %eax #push the parameter into register

    #check whether the cmd value is valid
    cmpl $3, %eax
    jg MP1_IOCTL_OVER #if %eax is greater than 3


    cmpl $0, %eax 
    jl MP1_IOCTL_OVER #if %eax is less than 0

    
    jmp *JUMP_TABLE(,%eax,4) #Each function address takes 4 bytes

    MP1_IOCTL_OVER:
        movl $-1, %eax
        ret


# int mp1 ioctl add(unsigned long arg)
# Interface: C style fucntion with calling convention
# Inputs: pointer to a struct
# Outputs: return 0 for success, return -1 for failure
# Registers:
#       %ebx store the return value of malloc
#       %edi store the return value of mp1_copy_from_user
#       %cx temporal register
#       %eax temporal register
mp1_ioctl_add:
    pushl %ebp
    movl  %esp, %ebp 
    #save callee saved registers
    pushl %ebx
    pushl %esi
    pushl %edi

    #eax: Store the value of arg and check whether its valid
    movl 8(%ebp), %eax
    cmpl $0, %eax
    je MALLOC_FAIL


    # Prepare to call malloc
    pushl %eax
    pushl %ecx
    pushl %edx

    pushl $STRUCT_SIZE #push the parameter onto the stack
    call mp1_malloc    #eax stores the return value

    addl $4, %esp #pop the parameter STRUCT_SIZE from the stack
    movl %eax, %ebx #store the return value of malloc function

    #pop the caller saved registers
    popl %edx
    popl %ecx
    popl %eax
    cmpl $0, %ebx 
    je MALLOC_FAIL 

    


    #prepare to run mp1_copy_from_user
    #(void *to, const void *from, unsigned long n)

    #push the caller saved registers
    pushl %eax
    pushl %ecx
    pushl %edx
    #push the arguments from right to left
    pushl $STRUCT_SIZE #size of the struct
    pushl 8(%ebp)      #from(pointer to the struct)
    pushl %ebx         #to
    call mp1_copy_from_user
    #pop the arguments
    addl $12, %esp #pop 3 arguments
    movl %eax, %edi #save the return value in %edi

    #pop the caller saved registers
    popl %edx
    popl %ecx
    popl %eax

    cmpl $0, %edi #check if copy succeed
    jne MP1_IOCTL_ADD_FAIL

    #check if location is valid
    movw (%ebx), %dx 
    cmpw $0, %dx #location < 0, then fail
    jl MP1_IOCTL_ADD_FAIL
    cmpw $1999, %dx #1999 is the upper bound of location given in the document
    jg   MP1_IOCTL_ADD_FAIL

    #move ON_LENGTH into the COUNTDOWN field
    movw ON_LENGTH(%ebx), %ax
    movw %ax, COUNTDOWN (%ebx)

    #change the status to 1
    movw $1, STATUS(%ebx)

    #insert struct at the head of the list
    movl mp1_list_head, %eax
    movl %eax, NEXT(%ebx)
    movl %ebx, mp1_list_head

    #no need for calling convention
    andl $0, %eax #clear %eax
    movzwl (%ebx), %eax

    movb ON_CHAR(%ebx), %cl #store the character to write
    pushl %edx
    call mp1_poke
    popl %edx
    movl $0, %eax #return for success

    #pop the callee saved registers
    popl %edi
    popl %esi
    popl %ebx
    leave
    ret

    MALLOC_FAIL:
        movl $-1, %eax

        #pop the callee saved registers
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
    #After malloc, the functions fails
    MP1_IOCTL_ADD_FAIL:
        #caller saved registers
        pushl %eax
        pushl %ecx
        pushl %edx

        pushl %ebx #argument
        call mp1_free
        addl $4, %esp #pop argument off stack


        popl %edx
        popl %ecx
        popl %eax

        movl $-1, %eax #store the return value in %eax

        #pop the callee saved registers
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret


# int mp1 ioctl remove(unsigned long arg)
# Interface: C style fucntion with calling convention
# Inputs: location of the target node in the linked list
# Outputs: return 0 for success, return -1 for failure
# Registers: 
#       %bx store the location from input
#       %edi store the return value from the saerch function
#       %esi temporal register
mp1_ioctl_remove:
    #calling convention
    pushl %ebp
    movl %esp, %ebp
    
    pushl %ebx
    pushl %esi
    pushl %edi

    #store the input into %bx
    movw 8(%ebp), %bx

    #check whether the input location is valid
    cmpw $0, %bx
    jb MP1_IOCTL_REMOVE_FAIL
    cmpw $1999, %bx #upper bound of the location value
    ja MP1_IOCTL_REMOVE_FAIL

    #prepare to call mp1_search_node
    pushl %eax
    pushl %ecx
    pushl %edx

    pushw %bx
    call  mp1_search_node
    movl  %eax, %edi
    addl  $2, %esp #pop %bx off the stack
    #pop caller saved registers
    popl  %edx
    popl  %ecx
    popl  %eax

    #check the return value of the search function
    cmpl $0, %edi
    je MP1_IOCTL_REMOVE_FAIL

    
    #check if it's a head pointer
    movl mp1_list_head, %esi
    cmpl %esi, %edi
    je   CUT_HEAD

    #find the predecessor of the node
    #%esi.next = %ebx

    FIND_PRE:    
        movl NEXT(%esi), %ebx 
        cmpl %edi, %ebx
        je CUT_NODE #find the predecessor
        cmpl $0, %esi
        je MP1_IOCTL_REMOVE_FAIL #no predecessor has been found until end
        movl NEXT(%esi), %esi
        jmp FIND_PRE

    CUT_HEAD:
        movl NEXT(%edi), %edx #store the next node of the list head into %edx
        movl %edx, mp1_list_head #make the second node the list head which is equivalent to cutting the list head
        jmp MP1_IOCTL_REMOVE_END

    CUT_NODE:
        movl NEXT(%edi), %edx #store the next node of the target node into %edx
        movl %edx, NEXT(%esi) #link the predecessor with the successor
        jmp MP1_IOCTL_REMOVE_END

    MP1_IOCTL_REMOVE_END:
        #prepare to call mp1_free
        #caller saved registers
        pushl %eax
        pushl %ecx
        pushl %edx
        pushl %edi #push argument
        call mp1_free
        addl $4, %esp #pop argument
        popl %edx
        popl %ecx
        popl %eax
        #store the return value
        movl $0, %eax
        #pop callee saved registers
        popl %edi
        popl %esi
        popl %ebx
        leave 
        ret

    MP1_IOCTL_REMOVE_FAIL:
        #store return value
        movl $-1, %eax
        #pop callee saved registers
        popl %edi
        popl %esi
        popl %ebx
        leave 
        ret
# int mp1 ioctl find(unsigned long arg)
# Interface: C style fucntion with calling convention
# Inputs: pointer to the target node in the linked list
# Outputs: return 0 for success, return -1 for failure
# Registers:
#       %ebx First store the pointer to input struct
#            Then store the return value of the mp1_search_node     
#       %esi First store the return value of the mp1_copy_from_user
#            Then store the return value of the mp1_copy_to_user
#       %edi store the malloced memory 
mp1_ioctl_find:
    pushl %ebp
    movl %esp, %ebp
    #callee saved registers
    pushl %ebx
    pushl %esi
    pushl %edi
    #copy the inputs into registers
    movl 8(%ebp), %ebx
    #check if the input pointer is valid
    cmpl $0, %ebx
    je MP1_IOCTL_FIND_FAIL

    #prepare to allcate memory for storing the struct copied struct to kernel space
    pushl %eax
    pushl %ecx
    pushl %edx

    pushl $STRUCT_SIZE #push the parameter onto the stack
    call mp1_malloc    #eax stores the return value

    addl $4, %esp #pop the parameter STRUCT_SIZE from the stack
    movl %eax, %edi #store the return value of malloc function

    #pop the caller saved registers
    popl %edx
    popl %ecx
    popl %eax
    #check the return value of the malloc function
    cmpl $0, %edi
    je MP1_IOCTL_FIND_FAIL_1

    #prepare to call copy_from_user and copy the struct into the newly allocated memory
    #push the caller saved registers
    pushl %eax
    pushl %ecx
    pushl %edx
    #push the arguments from right to left
    pushl $STRUCT_SIZE #size of the struct
    pushl %ebx      #from(pointer to the struct)
    pushl %edi         #to
    call mp1_copy_from_user
    #pop the arguments
    addl $12, %esp #pop 3 arguments
    movl %eax, %esi #save the return value in %esi

    #pop the caller saved registers
    popl %edx
    popl %ecx
    popl %eax
    #check if mp1_copy_from_user succeed
    cmpl $0, %esi
    jne MP1_IOCTL_FIND_FAIL_1

    #prepare to call mp1_search_node
    pushl %eax
    pushl %ecx
    pushl %edx

    pushw LOCATION(%edi) #The location field of the copied structure from input pointer
    call  mp1_search_node
    movl  %eax, %ebx
    addl  $2, %esp #pop %bx off the stack
    #pop caller saved registers
    popl  %edx
    popl  %ecx
    popl  %eax

    #check the return value of the search function
    cmpl $0, %ebx
    je MP1_IOCTL_FIND_FAIL_1


    #prepare to call mp1_copy_to_user
    pushl %eax
    pushl %ecx
    pushl %edx

    pushl $STRUCT_SIZE #push the argument 
    pushl %ebx         #from
    pushl 8(%ebp)      #to(the input pointer to the struct)
    call mp1_copy_to_user
    addl $12, %esp
    movl %eax, %esi #store the returned value in %esi
    popl  %edx
    popl  %ecx
    popl  %eax

    #check if mp1_copy_to_user return valid value
    cmpl $0, %esi
    je MP1_IOCTL_FIND_FAIL_1

    #prepare to call mp1_free
    pushl %eax
    pushl %ecx
    pushl %edx

    pushl %edi #argument
    call mp1_free
    addl $4, %esp #pop argument off stack

    popl %edx
    popl %ecx
    popl %eax

    #Success
    movl $0, %eax
    popl  %edi
    popl  %esi
    popl  %ebx
    leave
    ret
    #Fail at the malloc step, so no need to free
    MP1_IOCTL_FIND_FAIL:
        movl $-1, %eax

        #pop the callee saved registers
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret
    #Fail at other step, need to call mp1_free
    MP1_IOCTL_FIND_FAIL_1:
        movl $-1, %eax

        #prepare to call mp1_free
        pushl %eax
        pushl %ecx
        pushl %edx

        pushl %edi #argument
        call mp1_free
        addl $4, %esp #pop argument off stack

        popl %edx
        popl %ecx
        popl %eax

        #pop the callee saved registers
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret

# int mp1 ioctl sync(unsigned long arg)
# Interface: C style fucntion with calling convention
# Inputs: Two 16-bit integers
# Outputs: Return 0 on success, return -1 on failure
# Registers:   
#       %ebx store the return value of the mp1_search_node for first input location 
#       %esi store the return value of the mp1_search_node for second input location
#       %ax temporal register used for data transfer 
mp1_ioctl_sync:
    pushl %ebp
    movl %esp, %ebp
    #caller saved registers
    pushl %ebx
    pushl %esi
    pushl %edi
    #Initialize two local variables onto the stack
    xorl %eax, %eax
    pushl %eax
    pushl %eax
    #store the input into registers
    movw 8(%ebp), %si #second argument
    movw 10(%ebp), %bx #first argument
    #check whether these two locations are valid
    cmpw $0, %si
    jb MP1_IOCTL_SYNC_FAIL
    cmpw $1999, %si #1999 is the upper bound of location value
    ja MP1_IOCTL_SYNC_FAIL

    cmpw $0, %bx
    jb MP1_IOCTL_SYNC_FAIL
    cmpw $1999, %bx #1999 is the upper bound of location value
    ja MP1_IOCTL_SYNC_FAIL

    #Find the node with first input location
    #prepare to call mp1_search_node
    pushl %eax
    pushl %ecx
    pushl %edx

    pushw %bx
    call mp1_search_node
    addl $2, %esp
    movl %eax, %ebx

    popl  %edx
    popl  %ecx
    popl  %eax
    #check whether the return value of search of first location is valid
    cmpl $0, %ebx
    je MP1_IOCTL_SYNC_FAIL


    #Find the node with second input location
    #prepare to call mp1_search_node
    pushl %eax
    pushl %ecx
    pushl %edx

    pushw %si
    call mp1_search_node
    addl $2, %esp
    movl %eax, %esi

    popl  %edx
    popl  %ecx
    popl  %eax
    #check whether the return value of search of first location is valid
    cmpl $0, %esi
    je MP1_IOCTL_SYNC_FAIL

    #Now %ebx points to the first node, %esi points to the second node
    #prepare to copy ON_LENGTH, OFF_LENGTH, COUNTDOWN, STATUS into from first into second
    xorl %eax, %eax #clear %eax for later usage
    movw ON_LENGTH(%ebx), %ax
    movw %ax, ON_LENGTH(%esi)

    movw OFF_LENGTH(%ebx), %ax
    movw %ax, OFF_LENGTH(%esi)

    movw COUNTDOWN(%ebx), %ax
    movw %ax, COUNTDOWN(%esi)

    movw STATUS(%ebx), %ax
    movw %ax, STATUS(%esi)

    #check the status 
    cmpw $1, %ax
    je   SYNC_STATUS_ONE
    jmp  SYNC_STATUS_ZERO

    SYNC_STATUS_ONE:
        movzwl (%esi), %eax
        movb ON_CHAR(%esi), %cl
        #preserve the register for mp1_poke
        pushl %edx
        call mp1_poke
        popl %edx

        movl $0, %eax
        popl  %edi
        popl  %esi
        popl  %ebx
        leave 
        ret
    SYNC_STATUS_ZERO:
        movzwl (%esi), %eax
        movb OFF_CHAR(%esi), %cl
        #preserve the register for mp1_poke
        pushl %edx
        call mp1_poke
        popl %edx
        movl $0, %eax
        popl  %edi
        popl  %esi
        popl  %ebx
        leave 
        ret
    MP1_IOCTL_SYNC_FAIL:
        movl $-1, %eax
        popl  %edi
        popl  %esi
        popl  %ebx
        leave 
        ret


#jump table for the mp1_ioctl function
JUMP_TABLE:
    .long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync


#------------------helper functions------------------
# Search for one node in the list with specified location field
# Interface: C style fucntion with calling convention
# Inputs: location field of one struct
# Output: pointer to the found node, return NULL for failure
# Registers: 
#         %edi store the pointer to the current struct
#         %cx  temporal register
#         %esi store the input argument
mp1_search_node:
    pushl %ebp
    movl %esp, %ebp

    #push the callee saved registers
    pushl %ebx
    pushl %esi
    pushl %edi

    movl mp1_list_head, %edi #store pointer to struct
    movw 8(%ebp), %si
    KEEP_SEARCHING:
        #check if the current pointer is NULL. If it is, we have reached the end of linked list.
        cmpl $0, %edi
        je   REACH_NULL_PTR
        #store the location of the current node for checking
        movw LOCATION(%edi), %cx
        #compare with the target location value
        cmpw %cx, %si
        je   NODE_FOUND
        #update the pointer to the current checking node
        movl NEXT(%edi), %edi
        jmp KEEP_SEARCHING
    NODE_FOUND:
        #store the pointer to the target node into %eax for return
        movl %edi, %eax
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret

    REACH_NULL_PTR:
        movl $0, %eax #return 0 if fail
        popl %edi
        popl %esi
        popl %ebx
        leave
        ret






.end

