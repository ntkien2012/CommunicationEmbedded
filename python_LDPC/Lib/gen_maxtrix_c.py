import numpy as np

def generate_c_code_from_H(file_path: str, c_var_prefix: str = "H") -> None:
    """
    Load a dense H matrix from a .npy file, compute CSR (row_ptr, col_idx),
    and print corresponding C definitions to stdout.

    Args:
        file_path: Path to the H.npy file containing a dense 0/1 numpy array.
        c_var_prefix: Prefix for C variable names (e.g., "H" for row_ptr -> H_row_ptr).
    """
    # Load H
    H = np.load(file_path)
    m, n = H.shape

    # Compute CSR representation
    row_ptr = [0]
    col_idx = []
    for i in range(m):
        ones = np.nonzero(H[i])[0]
        col_idx.extend(ones.tolist())
        row_ptr.append(len(col_idx))
    nnz = len(col_idx)

    # Print C code
    print(f"// Generated from {file_path}")
    print(f"#define {c_var_prefix}_M   {m}    // number of rows in H")
    print(f"#define {c_var_prefix}_N   {n}    // number of columns in H")
    print(f"#define {c_var_prefix}_NNZ {nnz}   // number of non-zero entries in H\n")

    # row_ptr array
    print(f"static const uint16_t {c_var_prefix}_row_ptr[{c_var_prefix}_M+1] = {{")
    for i, val in enumerate(row_ptr):
        end = "," if i < len(row_ptr) - 1 else ""
        print(f"    {val}{end}")
    print("};\n")

    # col_idx array
    print(f"static const uint16_t {c_var_prefix}_col_idx[{c_var_prefix}_NNZ] = {{")
    for i, val in enumerate(col_idx):
        end = "," if i < len(col_idx) - 1 else ""
        # break line every 16 entries for readability
        if i % 16 == 0:
            print("    ", end="")
        print(f"{val}{end} ", end="")
        if (i + 1) % 16 == 0:
            print()
    if nnz % 16 != 0:
        print()
    print("};")

if __name__ == "__main__":
    # Example usage
    generate_c_code_from_H("H.npy", c_var_prefix="H")
