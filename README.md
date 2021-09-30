# Polynomials
Calculator for polynomials

This is a project for my university.

This calculator allows you to operate on multivariable polynomials. You can enter polynomials on the input console and then perform operations such as check if the last added polynomial is equal to zero or multiply two last added polynomials and then print the result.

In this progam, a polynomial has a recurrent definition. It is either a coefficient (that is, an integer value) or a sum of monomials. A monomial is a product of a variable raised to a power and a polynomial. Variables in a polynomial are numbered _x_i_, where _i_ is the depth of the recursion. You'll find more about the definition of a polynomial in the next paragraph and in the examples below.

The polynomials entered within the console are added on top of the stack which is empty at the beginning of the program. 
If you want to add a polynomial to the stack you need to enter it on the console in a format specified by following rules:

_POLYNOMIAL_ → _COEFFICIENT_ | _MONOMIALS_

_MONOMIALS_ → (_MONOMIAL_) | (_MONOMIAL_)+_MONOMIALS_

_MONOMIAL_ → _POLYNOMIAL_,_EXPONENT_

_COEFFICIENT_ and _EXPONENT_ need to be integers, like 2 or -193. _EXPONENT_ needs to be positive.


**Example inputs:**

(3,2)

_= x<sub>0</sub><sup>2</sup>*3_

(2,1)+(1,4) 

_= x<sub>0</sub>*2 + x<sub>0</sub><sup>4</sup>_

((5,2),3) 

_= x<sub>0</sub><sup>3</sup>*x<sub>1</sub><sup>2</sup>*5_

((4,1)+(2,3),1)+(1,2) 

_= x<sub>0</sub>*(x<sub>1</sub>*4 + x<sub>1</sub><sup>3</sup>*2) + x<sub>0</sub><sup>2</sup>_


Apart from adding a new polynomial to the stack, you can enter following commands on the console:
- ZERO - inserts a polynomial equal to zero at the top of the stack
- IS_COEFF - checks if the polynomial at the top of the stack is a coefficient, prints 0 or 1 adequately
- IS_ZERO - checks if the polynomial at the top of the stack is equal to zero, prints 0 or 1
- CLONE - inserts a copy of the polynomial at the top to the top of the stack
- ADD - adds two polynomials at the top of the stack, removes these two polynomials from the stack and inserts the sum
- MUL - multiplies two polynomials at the top of the stack, removes them and inserts the product
- NEG - negates the polynomial at the top of the stack
- SUB - subtracts second polynomial (from the top) from the polynomial at the top, removes them and inserts the difference
- IS_EQ - checks if two polynomials at the top of the stack are equal, prints 0 or 1
- DEG - prints the degree of the polynomial at the top (or -1 if it is equal to zero)
- DEG_BY _x_ - prints the degree of the polynomial at the top taking into account the variable _x_
- AT _y_ - computes the value of the polynomial at _y_, removes the polynomial from the stack and puts the result of the operation on the stack
- PRINT - prints the polynomial at the top of the stack
- POP - removes the polynomial at the top of the stack
- COMPOSE - performs top-of-stack polynomial composite with k consecutive top-of-stack polynomials, removes these k + 1 polynomials from stack, and inserts the result of composite on top of stack





