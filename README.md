# Polynomials
Calculator for polynomials

This calculator lets you operate on multivariable polynomials. You can enter polynomials on the input console and then perform operations such as check if the last added polynomial is equal to zero or multiply two last added polynomials.

In this progam, a polynomial has a recurrent definition. It is either a coefficient (that is, an integer value) or a sum of monomials. A monomial is a product of a variable raised to a power and a polynomial. Variables in a polynomial are numbered _x_i_, where _i_ is the depth of the recursion. You'll find more about the definition of a polynomial in the next paragraph and in the examples below.

The polynomials entered within the console are added on top of the stack which is empty at the beginning of the program. 
If you want to add a polynomial to the stack it has to have a format specified by following rules:
_polynomial_ → _coefficient_ | _monomials_
_monomials_ → (_monomial_) | (_monomial_)+_monomials_
_monomial_ → _polynomial_,_exponent_

Example inputs:
(3,2) 
= x_0^2*3
(2,1)+(1,4) 
= x_0*2 + x_0^4
((5,2),3) 
= x_0^3*x_1^2*5
((4,1)+(2,3),1)+(1,2) 
= x_0^1*(x_1*4 + x_1^3*2) + x_0^2



