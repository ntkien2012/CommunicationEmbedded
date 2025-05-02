import numpy as np
import matplotlib.pyplot as plt
from scipy import special
import sys
import random
sys.stdout.reconfigure(encoding='utf-8')

class SPA:
    """ This class can apply SPA algorithm to received LLR vector r.
    
    Parameters
    ----------
    H: numpy.array
        Parity-Check matrix.
    Imax: int, optional
        Maximum number of iterations.
    trace_on: bool, optional
        To print or not to print intermediate results of calculations.
    
    Attributes
    ----------
    H: 2D numpy.array
        Parity-Check matrix.
    Imax: int
        Maximum number of iterations.
    trace_on: bool
        To print or not to print intermediate results of calculations.
    H_0: int
        Number of rows of the Parity-Check matrix.
    H_1: int
        Number of columns of the Parity-Check matrix.
    H_mirr: 2D numpy.array:
        'Mirror' of the Parity-Check matrix.
    """
    
    def __init__(self, H, Imax=1000, trace_on=False):
        self.H = H
        self.Imax = Imax
        self.trace_on = trace_on
        self.H_0 = np.shape(H)[0]
        self.H_1 = np.shape(H)[1]
        self.H_mirr = (self.H + np.ones(np.shape(self.H))) % 2
    
    def __nrz(self, l):
        """Applies inverse NRZ 
        
        Parameters
        ----------
        l: 1D numpy.array
            LLR vector.
        
        Returns
        -------
        l: 1D numpy.array
            Mapped to binary symbols input vector.
        """
        l_copy = l.copy()  # Sử dụng bản sao để tránh sửa đổi đầu vào
        for idx, l_j in enumerate(l_copy):
            if l_j >= 0:
                l_copy[idx] = 0
            else:
                l_copy[idx] = 1
        return l_copy

    def __calc_E(self, E, M):
        """ Calculates V2C message 
        
        Parameters
        ----------
        E: 2D numpy.array
            Current V2C matrix.
        M: 2D numpy.array
            Current C2V matrix.
        
        Returns
        -------
        E: 2D numpy.array
            Updated V2C matrix.
        """
        M_tanh = np.tanh(M / 2)
        E_new = np.zeros_like(M)
        for j in range(self.H_0):
            idxs = np.where(self.H[j] != 0)[0]
            for i in idxs:
                others = idxs[idxs != i]
                prod = np.prod(M_tanh[j, others])
                prod = np.clip(prod, -0.999999, 0.999999)
                E_new[j, i] = 2 * np.arctanh(prod)
        return E_new

    def __calc_M(self, M, E, r):
        """ Calculates C2V message 
        
        Parameters
        ----------
        M: 2D numpy.array
            Current C2V matrix.
        E: 2D numpy.array
            Current V2C matrix.
        r: 1D numpy.array
            Input LLR vector.
        
        Returns
        -------
        M: 2D numpy.array
            Updated C2V matrix.
        """
        M_new = np.zeros_like(M)
        for j in range(self.H_0):
            for i in np.where(self.H[j] != 0)[0]:
                j_idxs = np.where(self.H[:, i] != 0)[0]
                s = np.sum(E[j_idxs, i]) - E[j, i]
                M_new[j, i] = r[i] + s
        return M_new

    def decode(self, r):
        """Applies SPA algorithm to received LLR vector r.
    
        Parameters
        ----------
        r: numpy.array of floats
            received from demodulator LLR vector.
        
        Returns
        -------
        l: numpy.array
            Decoded message.
        """
        stop = False  # stopping flag
        I = 0  # current iteration
        M = np.zeros(np.shape(self.H))  # C2V
        E = np.zeros(np.shape(self.H))  # V2C
        
        if self.trace_on:
            print('H:\n'+str(self.H))
        
        while stop == False and I != self.Imax:
            """ 1) Initial step """
            if I == 0:
                for j in range(self.H_0):
                    M[j, :] = r * self.H[j, :]
            
            if self.trace_on:
                print('M:\n'+str(M))
            
            """ 2) V2C step """
            E = self.__calc_E(E, M)
            
            if self.trace_on:    
                print('E:\n'+str(E))
            
            """ 3) Decoded LLR vector """
            l = r + np.sum(E, axis=0)
            
            if self.trace_on:
                print('l:\n'+str(l))
            
            """ 4) NRZ mapping """
            l_hard = self.__nrz(l)
            
            if self.trace_on:
                print('decoded:\n'+str(l_hard))
            
            """ 5) Syndrom checking """
            s = np.dot(self.H, l_hard) % 2
            if np.all(s == 0):
                stop = True
            else:
                I = I + 1
                M = self.__calc_M(M, E, r)
        
        return l_hard

def encode_ldpc(message, G):
    """
    Encrypt a message using the generator matrix G

    Parameters:
    -----------
    message: numpy.array
    Vector of information bits (0 or 1)
    G: numpy.array
    Generator matrix

    Returns:
    --------
    codeword: numpy.array
    Codeword to be encrypted
    """
    # Kiểm tra kích thước
    K = G.shape[0]  # Số bit thông tin
    if len(message) != K:
        raise ValueError(f"The message size ({len(message)}) must be equal to the number of rows of G ({K})")
    
    # Mã hóa: c = m·G
    codeword = np.dot(message, G) % 2
    
    return codeword

def bpsk_modulate(bits):
    """
    BPSK modulation: 0 -> +1, 1 -> -1

    Parameters:
    -----------
    bits: numpy.array
    Bit vector (0 or 1)

    Returns:
    --------
    symbols: numpy.array
    BPSK symbols
    """
    return 1 - 2 * bits  # 0 -> +1, 1 -> -1

def awgn_channel(symbols, snr_db):
    """
    AWGN channel simulation

    Parameters:
    -----------
    symbols: numpy.array
    Modulated symbols
    snr_db: float
    Signal to noise ratio (dB)

    Returns:
    --------
    received: numpy.array
    Received signal after passing through noisy channel
    """
    # Convert SNR from dB to linear
    snr_linear = 10 ** (snr_db / 10)
    
    # Calculate the noise variance (sigma^2)
    # With BPSK modulation, the average energy per bit is 1
    noise_var = 1 / (2 * snr_linear)
    sigma = np.sqrt(noise_var)
    
    # Generate Gaussian noise
    noise = sigma * np.random.normal(0, 1, len(symbols))
    
    # Add noise to the signal
    received = symbols + noise
    
    return received

def calculate_llr(received, snr_db):
    """
    Calculate LLR (Log-Likelihood Ratio) for each bit

    Parameters:
    -----------
    received: numpy.array
    Received signal after passing through noisy channel
    snr_db: float
    Signal to noise ratio (dB)

    Returns:
    --------
    llr: numpy.array
    LLR vector
    """
    # Convert SNR from dB to linear
    snr_linear = 10 ** (snr_db / 10)
    
    # Calculate the noise variance
    noise_var = 1 / (2 * snr_linear)
    
    # Calculate LLR: LLR = 2*r/sigma^2 with BPSK in AWGN channel
    llr = 2 * received / noise_var
    
    return llr

def ber_calculator(original, decoded):
    """
    Calculate Bit Error Rate (BER)

    Parameters:
    -----------
    original: numpy.array
    Original data
    decoded: numpy.array
    Decoded data

    Returns:
    --------
    ber: float
    Bit Error Rate
    """
    if len(original) != len(decoded):
        raise ValueError("The length of the original data and the decoded data must be equal.")
    
    errors = np.sum(original != decoded)
    total = len(original)
    
    return errors / total

def simulate_ldpc(H, G, num_frames=100, snr_db_range=np.arange(0, 11, 1)):
    """
    Simulate the performance of LDPC code over AWGN channel

    Parameters:
    -----------
    H: numpy.array
    Parity check matrix
    G: numpy.array
    Generator matrix
    num_frames: int
    Number of simulated frames for each SNR value
    snr_db_range: numpy.array
    Range of SNR values ​​(dB) to simulate

    Returns:
    --------
    ber_values: numpy.array
    BER values ​​corresponding to each SNR value
    """
    K = G.shape[0]  # Number of information bits
    N = G.shape[1]  # Number of encoding bits
    
    # Initialize a list to store BER results
    ber_values = []
    
    # Initialize SPA decryption object
    decoder = SPA(H, Imax=50, trace_on=False)
    
    # Simulation for each SNR value
    for snr_db in snr_db_range:
        total_errors = 0
        total_bits = 0
        
        print(f"Simulation with SNR = {snr_db} dB")
        
        for _ in range(num_frames):
            # Generate random message
            message = np.random.randint(0, 2, K)
            
            # Encryption
            codeword = encode_ldpc(message, G)
            
            # BPSK modulation
            symbols = bpsk_modulate(codeword)
            
            # Transmission over AWGN channel
            received = awgn_channel(symbols, snr_db)
            
            # LLR Calculation
            llr = calculate_llr(received, snr_db)
            
            # Decode
            decoded = decoder.decode(llr)
            
            # Compare only the information part (say the first K bits)
            # Note: Depending on the structure of the code, this may be different
            # If G is in normal form [I_K | P^T], then the first K bits of the codeword are the message
            errors = np.sum(message != decoded[:K])
            
            total_errors += errors
            total_bits += K
        
        # Calculate BER
        ber = total_errors / total_bits
        ber_values.append(ber)
        print(f"BER = {ber}")
    
    return np.array(ber_values)

def theoretical_uncoded_ber(snr_db):
    """
    Theoretical BER calculation for BPSK uncoded system on AWGN channel

    Parameters:
    -----------
    snr_db: numpy.array
    SNR range (dB)

    Returns:
    --------
    ber: numpy.array
    Theoretical BER
    """
    snr_linear = 10 ** (snr_db / 10)
    return 0.5 * special.erfc(np.sqrt(snr_linear))

def main():
    """
    Main function to perform simulation
    """
    try:
        # Read matrices H and G from .npy file
        H = np.load('H.npy')
        G = np.load('G.npy')
        
        print(f"Loaded matrix H size {H.shape}")
        print(f"Loaded matrix G size {G.shape}")
        
        # Check compatibility between H and G
        result = np.dot(G, H.T) % 2
        if np.sum(result) == 0:
            print("G·H^T = 0: Two compatible matrices")
        else:
            print("Warning: G·H^T ≠ 0, Two matrices may not be compatible!")
        
        # Set simulation parameters
        snr_db_range = np.arange(0, 11, 1)  # From 0 to 10 dB
        num_frames = 100  # Number of simulated frames per SNR
        
        # Perform simulation
        ber_values = simulate_ldpc(H, G, num_frames, snr_db_range)
        
        # Calculate the theoretical BER for the uncoded system for comparison.
        ber_uncoded = theoretical_uncoded_ber(snr_db_range)
        
        # Graph the results
        plt.figure(figsize=(10, 6))
        plt.semilogy(snr_db_range, ber_values, 'o-', label='LDPC Code')
        plt.semilogy(snr_db_range, ber_uncoded, 's--', label='Uncoded BPSK')
        plt.grid(True)
        plt.xlabel('SNR (dB)')
        plt.ylabel('Bit Error Rate (BER)')
        plt.title('BER Performance of LDPC Code over AWGN Channel')
        plt.legend()
        plt.savefig('ldpc_performance.png')
        plt.show()
        
    except FileNotFoundError:
        print("Error: File H.npy or G.npy not found")
        print("Please make sure these files are in the same folder as the script.pt")
    except Exception as e:
        print(f"Unexpected error: {e}")

def test_small_example():
    """
    Test the algorithm with a small example
    """

    H = np.load('H.npy')
    G = np.load('G.npy')
    
    # Check compatibility
    result = np.dot(G, H.T) % 2
    print("Kiểm tra G·H^T = ", result)
    
    # Create a message
    # message = np.array([1, 0, 1])
    message = np.random.randint(0, 2, 128)
    
    # Encryption
    codeword = encode_ldpc(message, G)
    print(f"Message: {message}")
    print(f"code: {codeword}")
    
    # BPSK modulation
    symbols = bpsk_modulate(codeword)
    
    # Add noise (SNR = 5dB)
    snr_db = 5
    received = awgn_channel(symbols, snr_db)
    
    # Calculate LLR
    llr = calculate_llr(received, snr_db)
    
    # Decode
    decoder = SPA(H, Imax=10, trace_on=True)
    decoded = decoder.decode(symbols)
    
    print(f"Received signal: {received}")
    print(f"Decoded code: {decoded}")
    print(f"Decoded information bits: {decoded[:128]}")
    print(f"Exactly: {np.array_equal(message, decoded[:128])}")

if __name__ == "__main__":
    # Uncomment to run small example
    # test_small_example()
    
    # Run main simulation
    main()