import numpy as np
from scipy.sparse import eye, bmat, csr_matrix
from numpy.linalg import matrix_rank
from sionna.phy.fec.ldpc.encoding import LDPC5GEncoder
import sys
sys.stdout.reconfigure(encoding='utf-8')

def protograph_qc_ldpc(B: np.ndarray, Z: int, seed: int = None) -> csr_matrix:
    """
    Generate QC-LDPC H matrix and record shift-offsets for each non-zero block.
    Returns:
        H : csr_matrix of shape (m_b*Z, n_b*Z)
        shifts : 2D numpy array of shape (m_b, n_b) with shift values or None
    """
    rng = np.random.default_rng(seed)
    m_b, n_b = B.shape
    blocks = []
    shifts = [[None] * n_b for _ in range(m_b)]
    
    for i in range(m_b):
        row_blocks = []
        for j in range(n_b):
            if B[i, j] > 0:
                shift = int(rng.integers(0, Z))
                shifts[i][j] = shift
                I = eye(Z, format='csr')
                P = csr_matrix(np.roll(I.toarray(), shift, axis=1))
                row_blocks.append(P)
            else:
                row_blocks.append(csr_matrix((Z, Z), dtype=int))
        blocks.append(row_blocks)
    H = bmat(blocks, format='csr')
    return H, np.array(shifts, dtype=object)


def get_systematic_form(H):
    """Convert matrix H to normal form [P | I_M]"""
    M, N = H.shape
    K = N - M  # Number of information bits
    
    # Copy the matrix without changing the original matrix
    H_copy = H.copy()
    
    # Perform Gaussian elimination on matrix H to obtain the form [P | I_M]
    for i in range(M):
        # Find the row with bit 1 in column K+i
        pivot_row = None
        for j in range(i, M):
            if H_copy[j, K+i] == 1:
                pivot_row = j
                break
                
        if pivot_row is None:
            # If no 1 bit is found in this column, permute the columns
            for j in range(K):
                if H_copy[i, j] == 1:
                    # Swap column j and column K+i
                    H_copy[:, [j, K+i]] = H_copy[:, [K+i, j]]
                    break
            pivot_row = i
            
        # Swap rows if needed
        if pivot_row != i:
            H_copy[[i, pivot_row]] = H_copy[[pivot_row, i]]
            
        # Remove other 1 bits in column K+i
        for j in range(M):
            if j != i and H_copy[j, K+i] == 1:
                H_copy[j] = (H_copy[j] + H_copy[i]) % 2  # XOR the rows
    
    return H_copy

def generate_G_from_H(H):
    """Generate matrix G from matrix H in normal form"""
    M, N = H.shape
    K = N - M  # Number of information bits
    
    # Convert H to standard form
    H_sys = get_systematic_form(H)
    
    # Extract matrix P
    P = H_sys[:, :K]
    
    # Create matrix G = [I_K | P^T]
    I_K = np.eye(K, dtype=int)
    P_T = P.T
    G = np.hstack((I_K, P_T))
    
    return G.astype(int)

def verify_LDPC_matrices(H, G=None):
    """Full compatibility check of H and G matrices"""
    M, N = H.shape
    K = N - M  # Number of information bits
    
    # Check the rank of matrix H
    H_rank = matrix_rank(H)
    if H_rank < M:
        print(f"Warning: Rank of H ({H_rank}) < M ({M}), matrix H may be redundant.")
    
    # If G is not given, generate G from H
    if G is None:
        G = generate_G_from_H(H)
    
    # Check the size of G
    if G.shape != (K, N):
        print(f"Error: The size of G ({G.shape}) is incorrect, it should be ({K}, {N}).")
        return False
    
    # Check G·H^T = 0
    compatibility, message = check_compatibility(G, H)
    print(message)
    
    return compatibility

def check_compatibility(G, H):
    """Check the compatibility between G and H by calculating G·H^T"""
    result = np.matmul(G, H.T) % 2
    
    # If G·H^T = 0, the two matrices are compatible.
    if np.sum(result) == 0:
        return True, "G·H^T = 0, two matrices are compatible."
    else:
        return False, "G·H^T ≠ 0, two incompatible matrices."


if __name__ == "__main__":
    # Base prototype example (2 rows, 3 columns)
    B = np.array([
        [1, 0, 1],
        [0, 1, 1],
    ], dtype=int)

    Z = 128   # lifting factor
    H, shift = protograph_qc_ldpc(B, Z, seed=42)
    # encoder = LDPC5GEncoder(k=128, n =171)
    # H = (encoder._bm != -1).astype(int)  

    print("H shape:", H.shape)      
    H = H.toarray()

    G = generate_G_from_H(H)
    verify_LDPC_matrices(H, G)
    compatible, message = check_compatibility(G, H)
    print(message)
    print(verify_LDPC_matrices(H, G))
    print(H)
    print("Shift-offsets for each non-zero block (None means zero-block):")
    print(shift)

    np.save("H.npy", H)
    np.save("G.npy", G)
