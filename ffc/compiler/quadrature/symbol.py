"This file implements a class to represent a symbol."

__author__ = "Kristian B. Oelgaard (k.b.oelgaard@tudelft.nl)"
__date__ = "2009-07-12 -- 2009-08-08"
__copyright__ = "Copyright (C) 2009 Kristian B. Oelgaard"
__license__  = "GNU GPL version 3 or any later version"

# FFC common modules.
#from ffc.common.log import error

from symbolics import type_to_string, create_float, create_product, create_fraction
from expr import Expr

class Symbol(Expr):
    __slots__ = ("v", "base_expr", "base_op")
    def __init__(self, variable, symbol_type, base_expr=None, base_op=0):
        """Initialise a Symbols object, it derives from Expr and contains
        the additional variables:

        v         - string, variable name
        base_expr - Other expression type like 'x*y + z'
        base_op   - number of operations for the symbol itself if it's a math
                    operation like std::cos(.) -> base_op = 1.
        NOTE: self._prec = 1."""

        # Dummy value, a symbol is always one.
        self.val = 1.0

        # Initialise variable, type and class.
        self.v = variable
        self.t = symbol_type
        self._prec = 1

        # Needed for symbols like std::cos(x*y + z),
        # where base_expr = x*y + z.
        # ops = base_expr.ops() + base_ops = 2 + 1 = 3
        self.base_expr = base_expr
        self.base_op = base_op

        # If type of the base_expr is lower than the given symbol_type change type.
        # TODO: Should we raise an error here? Or simply require that one
        # initalise the symbol by Symbol('std::cos(x*y)', (x*y).t, x*y, 1).
        if base_expr and base_expr.t < self.t:
            self.t = base_expr.t

        # Compute the representation now, such that we can use it directly
        # in the __eq__ and __ne__ methods (improves performance a bit, but
        # only when objects are cached).
        if self.base_expr:
            self._repr = "Symbol('%s', %s, %s, %d)" % (self.v, type_to_string[self.t], self.base_expr._repr, self.base_op)
        else:
            self._repr = "Symbol('%s', %s)" % (self.v, type_to_string[self.t])

        # Use repr as hash value.
        self._hash = hash(self._repr)

    # Print functions.
    def __str__(self):
        "Simple string representation which will appear in the generated code."
        return self.v

    # Binary operators.
    def __add__(self, other):
        "Addition by other objects."
        # NOTE: We expect expanded objects and we only expect to add equal
        # symbols, if other is a product, try to let product handle the addition.
        # TODO: Should we also support addition by other objects for generality?
        # Returns x + x -> 2*x, x + 2*x -> 3*x.
        if self._repr == other._repr:
            return create_product([create_float(2), self])
        elif other._prec == 2: # prod
            return other.__add__(self)
        raise RuntimeError("Not implemented.")

    def __mul__(self, other):
        "Multiplication by other objects."
        # NOTE: We assume expanded objects.
        # If product will be zero.
        if self.val == 0.0 or other.val == 0.0:
            return create_float(0)

        # If other is Sum or Fraction let them handle the multiply.
        if other._prec in (3, 4): # sum or frac
            return other.__mul__(self)

        # If other is a float or symbol, create simple product.
        if other._prec in (0, 1): # float or sym
            return create_product([self, other])

        # Else add variables from product.
        return create_product([self] + other.vrs)

    def __div__(self, other):
        "Division by other objects."
        # NOTE: We assume expanded objects.
        # If division is illegal (this should definitely not happen).
        if other.val == 0.0:
            raise RuntimeError("Division by zero.")

        # Return 1 if the two symbols are equal.
        if self._repr == other._repr:
            return create_float(1)

        # If other is a Sum we can only return a fraction.
        # TODO: Refine this later such that x / (x + x*y) -> 1 / (1 + y)?
        if other._prec == 3: # sum
            return create_fraction(self, other)

        # Handle division by FloatValue, Symbol, Product and Fraction.
        # Create numerator and list for denominator.
        num = [self]
        denom = []

        # Add floatvalue, symbol and products to the list of denominators.
        if other._prec in (0, 1): # float or sym
            denom = [other]
        elif other._prec == 2: # prod
            # Need copies, so can't just do denom = other.vrs.
            denom += other.vrs
        # fraction.
        else:
            # TODO: Should we also support division by fraction for generality?
            # It should not be needed by this module.
            raise RuntimeError("Did not expected to divide by fraction.")

        # Remove one instance of self in numerator and denominator if
        # present in denominator i.e., x/(x*y) --> 1/y.
        if self in denom:
            denom.remove(self)
            num.remove(self)

        # Loop entries in denominator and move float value to numerator.
        for d in denom:
            # Add the inverse of a float to the numerator, remove it from
            # the denominator and continue.
            if d._prec == 0: # float
                num.append(create_float(1.0/other.val))
                denom.remove(d)
                continue

        # Create appropriate return value depending on remaining data.
        # Can only be for x / (2*y*z) -> 0.5*x / (y*z).
        if len(num) > 1:
            num = create_product(num)
        # x / (y*z) -> x/(y*z),
        elif num:
            num = num[0]
        # else x / (x*y) -> 1/y.
        else:
            num = create_float(1)

        # If we have a long denominator, create product and fraction.
        if len(denom) > 1:
            return create_fraction(num, create_product(denom))
        # If we do have a denominator, but only one variable don't create a
        # product, just return a fraction using the variable as denominator.
        elif denom:
            return create_fraction(num, denom[0])
        # If we don't have any donominator left, return the numerator.
        # x / 2.0 -> 0.5*x.
        return num

    # Public functions.
    def get_unique_vars(self, var_type):
        "Get unique variables (Symbols) as a set."
        # Return self if type matches, also return base expression variables.
        s = set()
        if self.t == var_type:
            s.add(self)
        if self.base_expr:
            s.update(self.base_expr.get_unique_vars(var_type))
        return s

    def get_var_occurrences(self):
        """Determine the number of times all variables occurs in the expression.
        Returns a dictionary of variables and the number of times they occur."""
        # There is only one symbol.
        return {self:1}

    def ops(self):
        "Returning the number of floating point operation for symbol."
        # Get base ops, typically 1 for sin() and then add the operations
        # for the base (sin(2*x + 1)) --> 2 + 1.
        if self.base_expr:
            return self.base_op + self.base_expr.ops()
        return self.base_op

from floatvalue import FloatValue
from product    import Product
from sum_obj    import Sum
from fraction   import Fraction
