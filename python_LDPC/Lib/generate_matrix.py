import numpy as np
from numpy.linalg import matrix_rank
import sys
sys.stdout.reconfigure(encoding='utf-8')

def peg_construct(N, M, dv):
    """
    PEG construction of an M×N LDPC parity‐check matrix H
    with variable‐node degree profile dv (length N).
    """
    H = np.zeros((M, N), dtype=int)
    deg_C = np.zeros(M, dtype=int)   # current degree of each check‐node

    for v in range(N):
        # for each addition of the tth edge to variable-node v:
        for t in range(dv[v]):
            if t == 0:
                # First time: choose the check‐node with the smallest degree
                j_star = np.argmin(deg_C)
            else:
                # Expand the BFS tree from v to find the set of checks not in the tree.
                L = _peg_tree_expand(v, H, M)
                # Select the node with the smallest degree in L
                if len(L):
                    j_star = min(L, key=lambda j: deg_C[j])
                else:
                    # if L is empty, fallback to global argmin
                    j_star = np.argmin(deg_C)

            H[j_star, v] = 1
            deg_C[j_star] += 1

    return H

def _peg_tree_expand(v, H, M):
    """
    BFS from variable‐node v through H to find the set of unexplored check‐nodes. 
    Return the set of check‐nodes not in the BFS tree.
    """
    visited_V = {v}
    visited_C = set()
    frontier_V = {v}

    while True:
        # From frontier variable-nodes find adjacent check-nodes
        next_C = {
            j for u in frontier_V
            for j in np.nonzero(H[:, u])[0]
            if j not in visited_C
        }
        if not next_C:
            break
        visited_C |= next_C

        # From the checks just discovered, find the neighboring variable‐nodes
        frontier_V = {
            u for j in next_C
            for u in np.nonzero(H[j, :])[0]
            if u not in visited_V
        }
        if not frontier_V or len(visited_C) == M:
            break
        visited_V |= frontier_V

    # Return the set of unvisited check-nodes
    return set(range(M)) - visited_C

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


H =peg_construct(224,96,[2]*128+[5]*96)
# Generate matrix G from H
G = generate_G_from_H(H)

# Check compatibility
verify_LDPC_matrices(H, G)

# Or quick check
compatible, message = check_compatibility(G, H)
print(message)
print(verify_LDPC_matrices(H, G))
print(H)

np.save("H.npy", H)
np.save("G.npy", G)