from array import array

class BitStream(object):
    "Ad hoc BitStream, for testing."
    def __init__(self):
        self.buf = array('B')

    def write_true_n(self, n):
        for i in range(n):
            self.write_true()

    def write_false_n(self, n):
        for i in range(n):
            self.write_false()
    
    def write_true(self):
        self.buf.append(1)

    def write_false(self):
        self.buf.append(0)

