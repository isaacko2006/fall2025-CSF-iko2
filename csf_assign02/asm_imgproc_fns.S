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

	.globl emboss_gray
emboss_gray:
	/*TODO*/
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
	
	movl (%rdi), %eax 	/* get width and height */
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
	/* TODO: implement */
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
	/* TODO: implement */
	ret

/*
vim:ft=gas:
*/
