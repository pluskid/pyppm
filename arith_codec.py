class ArithCodec(object):
    """\
    Base class for ArithEncoder and ArithDecoder.
    """

    # Precision of coding
    CODE_VALUE_BITS = 27

    # The maximum interval border
    TOP_VALUE = (1 << CODE_VALUE_BITS) - 1

    # 1/4 of the largest range
    FIRST_QUARTER = TOP_VALUE/4 + 1

    # 1/2 of the largest interval
    HALF = 2*FIRST_QUARTER

    # 3/4 of the largest interval
    THIRD_QUARTER = HALF + FIRST_QUARTER

    def __init__(self):
        self.low = 0
        self.high = self.TOP_VALUE
