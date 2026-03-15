import csv
import sys

class IorTableEntry:
    def __init__(self, wavelength, n, k=0.0):
        # Convert micrometers to nanometers
        self.Wavelength = wavelength * 1000.0
        self.n = n
        self.k = k

    def __repr__(self):
        return f"IorTableEntry{{ {self.Wavelength}f, {self.n}f, {self.k}f }}"

def load_ior_table(csv_path):
    entries = []
    with open(csv_path, 'r', newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            wavelength = float(row['wl'])
            n = float(row['n'])
            k = float(row['k']) if 'k' in row and row['k'] else 0.0
            entries.append(IorTableEntry(wavelength, n, k))
    return entries

if __name__ == "__main__":
    csv_file = sys.argv[1]
    table_entries = load_ior_table(csv_file)
    count = len(table_entries)
    print(f"There are {count} IorTable entries")

    # Print as C/C++ array with count in brackets
    print(f"IorTableEntry table[{count}] =")
    print("{")
    for entry in table_entries:
        print(f"    {{ {entry.Wavelength}f, {entry.n}f, {entry.k}f }},")
    print("};")