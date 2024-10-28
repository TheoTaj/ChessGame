import re

# Read the file
file_path = 'valgrind_report.txt'
with open(file_path, 'r') as file:
    content = file.read()

# Find all blocks within curly braces
blocks = re.findall(r'\{[^}]*\}', content)

# Write the blocks to a new text file
output_path = 'curly_brace_blocks.txt'
with open(output_path, 'w') as output_file:
    for block in blocks:
        output_file.write(block + '\n\n')

output_path