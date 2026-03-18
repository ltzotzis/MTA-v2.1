def repeat_file_lines(input_file, output_file, n):
    # Open the input file and read all lines
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    # First line stays as header
    header = lines[0]
    
    # Rest of the lines to be repeated
    body = lines[1:]
    
    # Write to the output file
    with open(output_file, 'w') as f:
        f.write(header)  # write the first line
        for _ in range(n):
            f.writelines(body)  # write the rest repeated n times

# Example usage:
repeat_file_lines('ev6.ptrace', 'big.ptrace', 1000)
