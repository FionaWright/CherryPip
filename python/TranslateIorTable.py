import yaml
import sys

class IorTableEntry:
    def __init__(self, wavelength, n, k):
        # Convert micrometers to nanometers
        self.Wavelength = wavelength * 1000.0
        self.n = n
        self.k = k

    def __repr__(self):
        return f"IorTableEntry{{ {self.Wavelength}f, {self.n}f, {self.k}f }}"

def load_ior_table(yaml_path):
    with open(yaml_path, 'r') as f:
        content = yaml.safe_load(f)

    entries = []
    for dataset in content.get('DATA', []):
        if dataset.get('type') == 'tabulated nk':
            for line in dataset['data'].strip().splitlines():
                parts = line.split()
                if len(parts) == 3:
                    wavelength, n, k = map(float, parts)
                    entries.append(IorTableEntry(wavelength, n, k))
    return entries

if __name__ == "__main__":
    yaml_file = sys.argv[1]
    table_entries = load_ior_table(yaml_file)
    count = len(table_entries)
    print(f"There are {count} IorTable entries")

    # Print as C/C++ array with count in brackets
    print(f"IorTableEntry table[{count}] =")
    print("{")
    for entry in table_entries:
        print(f"    {{ {entry.Wavelength}f, {entry.n}f, {entry.k}f }},")
    print("};")