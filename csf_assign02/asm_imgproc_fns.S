/*
 * x86-64 assembly language implementations of functions
 */

	.section .text

/* Offsets of struct Image fields */
#define IMAGE_WIDTH_OFFSET   0
#define IMAGE_HEIGHT_OFFSET  4
#define IMAGE_DATA_OFFSET    8

/*
 *  Isolate the red pixel's bits
 *  Parameters:
 *  %edi - pixel
 */
	.globl get_r
get_r: 
	movl %edi, %eax
	/* shift the pixels down by 24 to isolate the red pixels */
	shrl $24, %eax
	andl $0xFF, %eax
	ret

/*
 *  Isolate the green pixel's bits
 *  Parameters:
 *  %edi - pixel
 */
 	.globl get_g
get_g: 
	movl %edi, %eax
	/* shift the pixels down by 16 to isolate the green pixels */
	shrl $16, %eax
	andl $0xFF, %eax
	ret

/*
 *  Isolate the blue pixel's bits
 *  Parameters:
 *  %edi - pixel
 */
 	.globl get_b
get_b: 
	movl %edi, %eax
	/* shift the pixels down by 8 to isolate the blue pixels */
	shrl $8, %eax
	andl $0xFF, %eax
	ret

/*
 *  Isolate the alpha pixel's bits
 *  Parameters:
 *  %edi - pixel
 */
	.globl get_a
get_a: 
	movl %edi, %eax
	/* Don't need to shift anything for the alpha pixels */
	andl $0xFF, %eax
	ret

/*
 * Helper function to do math behind emboss function to return correct, clamped gray value
 * 
 * Parameters:
 *   %edi - current pixel
 *   %esi - upper-left neighbor pixel
 *
 * Returns:
 *   %eax - gray value (0..255)
 */
	.globl emboss_gray
emboss_gray:
        .globl emboss_gray
emboss_gray:
	/* Registers used:
		* %r12d = diffr = (nr - r)
		* %r13d = diffg = (ng - g)
		* %r14d = diffb = (nb - b)
		* %r15d = candidate signed difference
		* %r8d  = temporary variable for absolute diff of candidate
		* %edx  = temp var for |diffg|
		* %ecx  = temp var for |diffb|
		* %eax  = return value (gray), also used by get_* helpers
		*/
	
    pushq %rbp
    movq %rsp, %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movl %edi, %r8d		/* move current pixel into r8 */
    movl %esi, %r9d 	/* move upper left neighbor into r9 */

	/* calculate diffr */
    movl %r9d, %edi	/*move neighbor into edi */
    call get_r 	/* get r of neighbor */
    movl %eax, %r12d 	/*r12d = nr */
    movl %r8d, %edi 	/* move pixel into edi */
    call get_r 		/* eax = R(pixel) */
    subl %eax, %r12d 	/*r12d = nr - r */

	/* calculate diffb */
    movl %r9d, %edi 	/* move neighbor into edi */
    call get_g		/*eax = G(neighbor) */
    movl %eax, %r13d 	/* r13d = ng */
    movl %r8d, %edi		/*move pixel into edi */
    call get_g	/*eax = G(pixel) */
    subl %eax, %r13d	/*r13d = ng - g*/

    movl %r9d, %edi		/* move neighbor into edi */
    call get_b		/* eax = B(neighbor) */
    movl %eax, %r14d	/* r14d = nb */
    movl %r8d, %edi 	/* move pixel into edi */
    call get_b	/*eax = B(pixel) */
    subl %eax, %r14d 	/*r14d = nb - b */
	
	/* select the difference with the greatest absolute value */
    movl %r12d, %r15d	/* r15 becomes the candidate */
    movl %r12d, %r8d	/* r8d = |candidate */
    testl %r8d, %r8d	/* check the sign of candidate */
    jge 1f	/* if it its greater than zero, skip the negation, otherwise negate */
    negl %r8d
1:
    movl %r13d, %edx	/* move diffg into edx */
    testl %edx, %edx
    jge 2f
    negl %edx /* edx holds |diffg| */
2:
    cmpl %r8d, %edx      /* compare |diffg| to the candidate */
    jle 3f	/* if |diffg| <= candidate, keep the current candidate */
	movl  %r13d, %r15d /* otherwise, set candidate = diffg */
    movl %edx,  %r8d   /* set the new candidate's absolute value equal to diff g*/
3:
    movl %r14d, %ecx   /* move diffb into ecx */
    testl %ecx, %ecx
    jge 4f
    negl %ecx	/* ecx = |diffb| */
4:
    cmpl %r8d, %ecx		/* compare |diffb| to candidate */
    jle 5f	/* if |diffb| <= candidate, keep the current candidate */
    movl %r14d, %r15d   /* otherwise, set candidate = diffb */
    movl %ecx, %r8d 	/* set the new candidate's absolute value equal to diff b*/
5:
	/* begin clamping */
    addl $128, %r15d	/* gray = 128 + diff */
    cmpl $0, %r15d	/* if gray < 0 */
    jge 6f
    xorl %r15d, %r15d /* gray = 0 */
    jmp 7f
6:
    cmpl $255, %r15d /*else if gray > 225 */
    jle 7f
    movl $255, %r15d /* gray = 225 */
7:
    movl %r15d, %eax /* move gray into eax */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    ret


/*
 * Definitions of image transformation functions
 */

/*
 *  Transform the color component values in each input pixel
 *  by applying the bitwise complement operation. I.e., each bit
 *  in the color component information should be inverted
 *  (1 becomes 0, 0 becomes 1.) The alpha value of each pixel should
 *  be left unchanged.
 * 
 *  Parameters:
 *  %rdi - pointer to the input Image
 *  %rsi - pointer to the output Image (in which the
 *         transformed pixels should be stored)
 */
	.globl imgproc_complement
imgproc_complement:
	/* Registers used:
	 * %r8d = total pixels
	 * %r9 = pointer to input->data
	 * %r10 = pointer to output->data
	 * %r11d = loop counter
     * %edx  = at first, used to track current pixel but later tracks alpha pixel
     * %ecx  = red pixels
     * %esi  = green pixels
     * %ebx  = blue pixels
	*/

	/* for this implementation, we will use only caller saved registers */
	pushq %rbp
	movq %rsp, %rbp
	
	movl (%rdi), %eax	/* get width and height */
	movl 4(%rdi), %ecx  	/* offset for four to get the height */
	imull %ecx, %eax  	/* multiply height and width to find total pixel count */
	movl %eax, %r8d /* r8d now holds the total number of pixels */

	/* move input and output image pointers to different register */
	movq 8(%rdi), %r9
	movq 8(%rsi), %r10
	
	xorl %r11d, %r11d /* set the counter for the loop index */

.Ltop: 
	/* break the loop once total pixels are accounted for */
	cmpl %r8d, %r11d
	jge .Lbottom

	/* isolate a single pixel, r9 + r11 * 4 gives index */
	movl (%r9, %r11, 4), %edx 

	movl %edx, %edi
	call get_r
	movl %eax, %ecx /* ecx holds the red pixel */

	movl %edx, %edi
	call get_g
	movl %eax, %esi /* esi holds the green pixel */

	movl %edx, %edi
	call get_b
	movl %eax, %ebx /* ebx holds the blue pixel */

	movl %edx, %edi
	call get_a
	movl %eax, %edx /* edx now holds the alpha pixel */

	/* invert the pixels */
	xorl $0xFF, %ecx
    xorl $0xFF, %esi
    xorl $0xFF, %ebx

	/* shift the bits back to the original position by respective bits */
	shll $24, %ecx
	shll $16, %esi
	shll $8, %ebx

	/* assemble the bits together */
	movl %ecx, %eax /* put red into pixel */
    orl %esi, %eax /* put green into pixel */
    orl %ebx, %eax /* put blue into pixel */
    orl %edx, %eax /* put alpha into pixel */

	movl %eax, (%r10,%r11,4) 	/* store the pixel back into the output image */
	
	incl %r11d
    jmp .Ltop

.Lbottom:
	popq %rbp
	ret

/*
 *  Transform the input image by swapping the row and column
 *  of each source pixel when copying it to the output image.
 *  E.g., a pixel at row i and column j of the input image
 *  should be copied to row j and column i of the output image.
 *  Note that this transformation can only be applied to square
 *  images (where the width and height are identical.)
 * 
 *  Parameters:
 *  %rdi - pointer to the input Image
 *  %rsi - pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 * 
 *  @return 1 if the transformation succeeded, or 0 if the
 *          transformation can't be applied because the image
 *          width and height are not the same
 */
	.globl imgproc_transpose
imgproc_transpose:
	/* Registers used:
	 * %eax - width of input
	 * %ecx - height of input
	 * %edx - i * width + j (inputIdx equivalent from c implementation)
	 * ebx - temp var to hold 1 pixel
	 * %r9 - input->data array
	 * %r10 - output->data array 
	 * %r11d - counter for row iteration
	 * %r12d - counter for col iteration
	 * %r13d - j * height + i (outputIdx equivalent from c implementation)
	 * %rdx - 64 bit version of edx
	 */

	/* push registers onto stack */
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	pushq %r13
	
	movl (%rdi), %eax /* get width */
	movl 4(%rdi), %ecx /* offset for four to get the height */

	cmpl %ecx, %eax /* check if height != width */
	jne .height_not_equal_width

	movl %eax, (%rsi) 	/* set output image's width */
	movl %ecx, 4(%rsi) /* set output image's height */

	movq 8(%rdi), %r9 /* load input->data */
	movq 8(%rsi), %r10 /* load output->data */

	xorl %r11d, %r11d /*set a counter to zero to track height */

	.row_iterate: /*iterate through the rows */
		cmpl %ecx, %r11d /* compare counter to the height */
		jge .exit_loop
		
		xorl %r12d, %r12d /*set a counter to zero to track width */
	.col_iterate: /* iterate through the cols */
		cmpl %eax, %r12d /* compare counter to width */
		jge .go_to_next_row

		movl %r11d, %edx /* move row counter to edx */
		imull %eax, %edx /* multiply width with row counter */
		addl %r12d, %edx /* add width counter to the above to get i * width + j */

		movl %r12d, %r13d /*move width counter to r13 */
		imull %ecx, %r13d /* multiply height with width counter */
		addl %r11d, %r13d /* add height counter to the above to get j * height + i */

		movl (%r9, %rdx, 4), %ebx /* set temp variable to input->data[index] */ 
		/* we use rdx to account for the 64bit index register */
		movl %ebx, (%r10, %r13, 4) /* set output->data[index] to the temp variable above */
		/* we use ebx to account for the 64bit index register */ 

		incl %r12d /* increment the width counter */
		jmp .col_iterate

	.height_not_equal_width: /* terminate program if height and width are not equal */
		movl $0, %eax /* the image is not a square, return 0 */
		popq %r13
		popq %r12
		popq %rbx
		movq %rbp, %rsp
		popq %rbp
		ret

	.go_to_next_row:
		incl %r11d /* increment the height counter */
		jmp .row_iterate
	
	.exit_loop:
		movl $1, %eax /* loop successful, return 1 */
		popq %r13
		popq %r12
		popq %rbx
		movq %rbp, %rsp
		popq %rbp
		ret


/*
 *  Transform the input image by copying only those pixels that are
 *  within an ellipse centered within the bounds of the image.
 *  Pixels not in the ellipse should be left unmodified, which will
 *  make them opaque black.
 * 
 *  Let w represent the width of the image and h represent the
 *  height of the image. Let a=floor(w/2) and b=floor(h/2).
 *  Consider the pixel at row b and column a is being at the
 *  center of the image. When considering whether a specific pixel
 *  is in the ellipse, x is the horizontal distance to the center
 *  of the image and y is the vertical distance to the center of
 *  the image. The pixel at the coordinates described by x and y
 *  is in the ellipse if the following inequality is true:
 * 
 *    floor( (10000*x*x) / (a*a) ) + floor( (10000*y*y) / (b*b) ) <= 10000
 * 
 *  Parameters:
 *  %rdi - pointer to the input Image
 *  %rsi - pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 */
	.globl imgproc_ellipse
imgproc_ellipse:
	/* Registers used:
	 * %eax - width of input image, then division numerator
	 * %ecx - height of input image
	 * %r8 - variable a = width / 2
	 * %r9 - variable b = height / 2
	 * %r10 - height iteration counter (i equivalent)
	 * %r11 - row iteration counter (j equivalent)
	 * %r12 - x = j - a;
	 * %r13 - y = i - b;
	 * %r14 - ellipse calculation variable 
	 * %r15 - ellipse calculation variable
	 * %rbx - holds term 1 calculation
	 * %edx - denominator used for division
	 * %ebx - holds term 1 after division
	 * %rax - input data pointer
	 * %rcx - output data pointer
	 */
	 
	pushq %rbp
	movq %rsp, %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	pushq %rbx

	movl (%rdi), %eax /* get width */
	movl 4(%rdi), %ecx /* offset for four to get the height */

	movl %eax, %r8d /* move width into %r8d */
	shrl $1, %r8d /* perform bitwise shift right by 1 bit, dividing width by 2 */

	movl %ecx, %r9d /*move height into %r9d */
	shrl $1, %r9d /* perform bitwise shift right by 1 bit, dividing height by 2 */

	xorl %r10d, %r10d /* create a counter to track height iteration */
	.row_it:
		movl 4(%rdi), %ecx
		cmpl %ecx, %r10d /* compare height to height counter */
		jge .done
	
		xorl %r11d, %r11d /*set a counter to zero to track width */
	.col_it:
		movl (%rdi), %eax
		cmpl %eax, %r11d
		jge .next_r

		movl %r11d, %r12d /* move j into r12 */
		subl %r8d, %r12d /* subtract j from a */

		movl %r10d, %r13d /* move i into r13 */
		subl %r9d, %r13d /* subtract i from b */

		/* begin calcuation here */

		/* first term */
		movl %r12d, %r14d /* move x into r14d */
		imull %r14d, %r14d /* r14 = x * x */
		imull $10000, %r14d /* r14 = 10,000 * x */
		movl %r8d, %r15d /* r15d = a */
		imull %r15d, %r15d /* r15 = a * a */
		movl %r14d, %eax /* move r14d into the numerator */
		cdq
		idivl %r15d /* complete division */
		movl %eax, %ebx /*save term 1 into ebx */

		/* second term */
		movl %r13d, %r14d /* move y into r14d */
		imull %r14d, %r14d /* r14 = y * y */
		imull $10000, %r14d /* r14 = 10,000 * y */
		movl %r9d, %r15d /* r15d = b */
		imull %r15d, %r15d /* r15 = b * b */
		movl %r14d, %eax /* move r14d into the numerator */
		cdq
		idivl %r15d /* complete division */
		movl %eax, %r14d /*save term 1 into r14 */

		addl %ebx, %r14d /* add term 1 and term 2 */
		cmpl $10000, %r14d /*check if term1+term2 <= 10,000 */
		jg .make_pixel_black

		/* copy pixel data from input to output */
		movl (%rdi), %edx /* width */
		imull %r10d, %edx /* i * width */
		addl %r11d, %edx /* add j to the above */
		movq 8(%rdi), %rax /* put input_img->data to rax */
    	movq 8(%rsi), %rcx /* put output_img->data to rcx */
		movl (%rax,%rdx,4), %r12d /* load pixel */
		movl %r12d, (%rcx,%rdx,4) /* store pixel */

		incl %r11d /*increment col iterator (j++) */
		jmp .col_it

	.make_pixel_black:
		movl (%rdi), %edx /* put width into edx */
		imull %r10d, %edx /* i * width */
		addl %r11d, %edx /* add j to the above */
		movq 8(%rsi), %rcx /* put output_img to rcx */
		movl $0x000000FF, (%rcx,%rdx,4)
		incl %r11d /* increment j */
		jmp .col_it 

	.next_r:
		incl %r10d /* increment height counter (i++) */
		jmp .row_it

	.done:
		popq %rbx
		popq %r15
		popq %r14
		popq %r13
		popq %r12
		popq %rbp
		ret

	ret

/*
 *  Transform the input image using an "emboss" effect. The pixels
 *  of the source image are transformed as follows.
 * 
 *  The top row and left column of pixels are transformed so that their
 *  red, green, and blue color component values are all set to 128,
 *  and their alpha values are not modified.
 * 
 *  For all other pixels, we consider the pixel's color component
 *  values r, g, and b, and also the pixel's upper-left neighbor's
 *  color component values nr, ng, and nb. In comparing the color
 *  component values of the pixel and its upper-left neighbor,
 *  we consider the differences (nr-r), (ng-g), and (nb-b).
 *  Whichever of these differences has the largest absolute value
 *  we refer to as diff. (Note that in the case that more than one
 *  difference has the same absolute value, the red difference has
 *  priority over green and blue, and the green difference has priority
 *  over blue.)
 * 
 *  From the value diff, compute the value gray as 128 + diff.
 *  However, gray should be clamped so that it is in the range
 *  0..255. I.e., if it's negative, it should become 0, and if
 *  it is greater than 255, it should become 255.
 * 
 *  For all pixels not in the top or left row, the pixel's red, green,
 *  and blue color component values should be set to gray, and the
 *  alpha value should be left unmodified.
 * 
 *  Parameters:
 *  %rdi - pointer to the input Image
 *  %rsi - pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 */
	.globl imgproc_emboss
imgproc_emboss:
	/* Registers used:
	 * %r8   - input->data pointer
	 * %r9   - output->data pointer
	 * %r10  - row index i
	 * %r11  - column index j
	 * %r12d - width
	 * %r13d - height
	 * %r14  - current pixel index
	 * %r15d - alpha value / temporary
	 * %eax, %ebx, %ecx, %edx - temporary for pixel assembly
	 */
	pushq %rbp
	movq %rsp, %rbp
	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	movl (%rdi), %r12d       /* width */
	movl 4(%rdi), %r13d      /* height */
	movq 8(%rdi), %r8        /* input->data */
	movq 8(%rsi), %r9        /* output->data */

	xorq %r10, %r10          /* row index i = 0 */

.row_loop:
	cmpl %r13d, %r10d
	jge .finished

	xorq %r11, %r11          /* column index j = 0 */

.col_loop:
	cmpl %r12d, %r11d
	jge .next_row

	/* current pixel index = i*width + j */
	movq %r10, %rax
	imulq %r12, %rax
	addq %r11, %rax
	movq %rax, %r14          /* r14 = index */

	movl (%r8,%r14,4), %ebx  /* load current pixel */

	/* edge pixels: top row or left column */
	test %r10d, %r10d
	jz .edge
	test %r11d, %r11d
	jz .edge

	/* non-edge pixel emboss */
	/* compute neighbor index = (i-1)*width + (j-1) */
	movq %r10, %rax
	dec %rax
	imulq %r12, %rax
	movq %r11, %rcx
	dec %rcx
	addq %rax, %rcx
	movl (%r8,%rcx,4), %esi  /* neighbor pixel */

	movl %ebx, %edi          /* current pixel */
	movl %esi, %esi          /* neighbor pixel */
	pushq %r8
	pushq %r9
	call emboss_gray
	popq  %r9
    popq  %r8

	/* preserve alpha of current pixel */
	movl %ebx, %edi
	pushq %r8
	pushq %r9
	call get_a               
	movl %eax, %r15d         /* save alpha */
	popq  %r9
    popq  %r8

	/* assemble gray into RGB */
	movl %eax, %ebx          /* gray for B */
	movl %eax, %ecx          /* gray for G */
	movl %eax, %edx          /* gray for R */
	shll $16, %edx           /* R = gray << 16 */
	shll $8, %ecx            /* G = gray << 8 */
	orl %ecx, %edx
	orl %ebx, %edx
	movl %r15d, %ebx
	shll $24, %ebx           /* alpha << 24 */
	orl %ebx, %edx           /* add alpha */
	movl %edx, (%r9,%r14,4)  /* store pixel */

	jmp .inc

.edge:
	/* edge pixel: set RGB = 128, preserve alpha */
	movl %ebx, %edi
	pushq %r8
    pushq %r9
    call get_a                  /* A -> %eax */
    popq  %r9
    popq  %r8
	movl %eax, %r15d

	movl $128, %edx
	shll $16, %edx           /* R = 128 << 16 */
	movl $128, %ecx
	shll $8, %ecx            /* G = 128 << 8 */
	orl %ecx, %edx
	movl $128, %ebx           /* B = 128 */
	orl %ebx, %edx
	movl %r15d, %ebx
	shll $24, %ebx           /* alpha << 24 */
	orl %ebx, %edx
	movl %edx, (%r9,%r14,4)  /* store pixel */

.inc:
	inc %r11d
	jmp .col_loop

.next_row:
	inc %r10d
	jmp .row_loop

.finished:
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbx
	movq %rbp, %rsp
	popq %rbp
	ret

/*
vim:ft=gas:
*/