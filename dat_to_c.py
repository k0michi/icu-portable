import sys
import os

def main():
    if len(sys.argv) < 4:
        print('Usage: python dat_to_c.py <input.dat> <output.c> <symbol_name>')
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    symbol_name = sys.argv[3]

    if not os.path.exists(input_path):
        print(f'Error: Input file {input_path} not found.')
        sys.exit(1)

    with open(input_path, 'rb') as f:
        data = f.read()

    with open(output_path, 'w') as f:
        f.write('#include "unicode/utypes.h"\n')
        f.write(f'U_EXPORT const struct {{\n')
        f.write(f'    double bogus;\n')
        f.write(f'    uint8_t bytes[{len(data)}];\n')
        f.write(f'}} {symbol_name} = {{\n')
        f.write(f'    0.0,\n')
        f.write(f'    {{\n')
        
        chunk_size = 16
        for i in range(0, len(data), chunk_size):
            chunk = data[i:i+chunk_size]
            hex_str = ', '.join(f'0x{b:02X}' for b in chunk)
            f.write(f'        {hex_str},\n')
            
        f.write('    }\n')
        f.write('};\n')

if __name__ == '__main__':
    main()
