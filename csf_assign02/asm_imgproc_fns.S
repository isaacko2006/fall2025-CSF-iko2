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
	.globl imgproc_complement
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
	.globl imgproc_complement
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
	.globl imgproc_complement
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
	.globl imgproc_complement
get_a: 
	movl %edi, %eax
	/* Don't need to shift anything for the alpha pixels */
	andl $0xFF, %eax
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
	
	/* get width and height */
	movl (%rdi), %eax
	/* offset for four to get the height */
	movl 4(%rdi), %ecx 
	/* multiply height and width to find total pixel count */
	imull %ecx, %eax 
	movl %eax, %r8d #r8d now holds the total number of pixels

	/* move image pointers to different register */
	movq 8(%rdi), %r9
	movq 8(%rsi), %r10
	
	xorl %r11d, %r11d #set the counter for the loop index

.Ltop: 
	/* break the loop once total pixels are accounted for */
	cmpl %eax, %r11d
	jge .Lbottom

	/* isolate a single pixel, r9 + r11 * 4 gives index */
	movl (%r9, %r11, 4), %edx 

	movl %edx, %edi
	call get_r
	movl %eax, %ecx /* edx holds the red pixel */

	movl %edx, %edi
	call get_g
	movl %eax, %esi /* esi holds the green pixel */

	movl %edx, %edi
	call get_b
	movl %eax, %ebx /* ebx holds the blue pixel */

	movl %edx, %edi
	call get_a
	movl %eax, %edx /* edx holds the alpha pixel */

	/* invert the pixels */
	xorl $0xFF, %ecx
    xorl $0xFF, %esi
    xorl $0xFF, %ebx

	/* shift the bits back to the original position */
	shll $24, %ecx
	shll $16, %esi
	shll $8, %ebx

	/* assemble the bits together */
	movl %ecx, %eax
    orl %esi, %eax
    orl %ebx, %eax
    orl %edx, %eax

	/* store the pixel back into the output image */
	movl %eax, (%r10,%r11,4) 
	
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
	/* TODO: implement */
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
