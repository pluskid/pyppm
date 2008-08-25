class ArithCodec(object):
    """\
    Base class for ArithEncoder and ArithDecoder.
    """

    # Number of bits for the range
    TOP_VALUE_BITS = 28

    # Number of bits to ignore when testing equality
    APPROXIMATE_EQUALITY_PRECISION_BITS = 8

    # If the difference between two numbers are less than
    # this, they are considered equal
    APPROXIMATE_EQUALITY_PRECISION = 1 << APPROXIMATE_EQUALITY_PRECISION_BITS

    # The maximum interval border
    TOP_VALUE = 1 << TOP_VALUE_BITS

    # 1/4 of the largest range
    FIRST_QUARTER = TOP_VALUE/4

    # 1/2 of the largest interval
    HALF = 2*FIRST_QUARTER

    # 3/4 of the largest interval
    THIRD_QUARTER = HALF + FIRST_QUARTER

    def __init__(self):
        self.low = 0
        self.high = self.TOP_VALUE

    def approximate_ge(self, a, b):
        """\
        Test if a is arrpoximately greater than or equal to b.
        """
        diff = a-b
        if diff >= -self.APPROXIMATE_EQUALITY_PRECISION:
            return True
        return False
