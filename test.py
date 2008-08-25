from arith_encoder import ArithEncoder
from bit_stream import BitStream

out = BitStream()
encoder = ArithEncoder(out)
encoder.encode(4, 9, 10)
encoder.encode(4, 9, 10)
encoder.encode(4, 9, 10)
encoder.encode(9, 10, 10)

print out.buf
