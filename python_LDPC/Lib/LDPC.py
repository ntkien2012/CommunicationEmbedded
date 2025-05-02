import numpy as np
import matplotlib.pyplot as plt
import sys
import random
sys.stdout.reconfigure(encoding='utf-8')
from typing import Tuple, List

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
    Mã hóa một thông điệp sử dụng ma trận sinh G
    
    Parameters:
    -----------
    message: numpy.array
        Vector bit thông tin (0 hoặc 1)
    G: numpy.array
        Ma trận sinh
    
    Returns:
    --------
    codeword: numpy.array
        Từ mã được mã hóa
    """
    # Kiểm tra kích thước
    K = G.shape[0]  # Số bit thông tin
    if len(message) != K:
        raise ValueError(f"Kích thước thông điệp ({len(message)}) phải bằng số hàng của G ({K})")
    
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
    AWGN Channel Simulation

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
    
    # Calculate noise variance (sigma^2)
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
        Vector LLR
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

class BitFlipDecoder:
    def __init__(self, H: np.ndarray, max_iter: int = 50, threshold: np.ndarray = None, seed: int = None):
        self.H = H.copy() % 2
        self.m, self.n = self.H.shape
        self.k = self.n - self.m           # số bit thông tin
        self.max_iter = max_iter
        self.col_weights = np.sum(self.H, axis=0)
        if threshold is None:
            self.threshold = np.floor(self.col_weights / 2).astype(int)
        elif np.isscalar(threshold):
            self.threshold = np.full(self.n, int(threshold), dtype=int)
        else:
            arr = np.array(threshold, dtype=int)
            if arr.size != self.n:
                raise ValueError("Threshold array must match number of bits (n)")
            self.threshold = arr
        self.rng = np.random.default_rng(seed)

    def decode(self, received: np.ndarray) -> np.ndarray:
        """
        Decode and return only the original k information bits.
        """
        decoded = received.copy() % 2

        for _ in range(self.max_iter):
            # Syndrome
            syndrome = self.H.dot(decoded) % 2
            if not syndrome.any():  # hội tụ
                return decoded[:self.k]

            # Đếm số unsatisfied checks
            unsat_counts = np.sum(self.H * syndrome[:, None], axis=0)

            # Static-phase
            to_flip = np.where(unsat_counts > self.threshold)[0]

            # Dynamic-phase nếu cần
            if to_flip.size == 0:
                max_unsat = unsat_counts.max()
                candidates = np.where(unsat_counts == max_unsat)[0]
                chosen = self.rng.choice(candidates)
                to_flip = np.array([chosen])

            # Flip
            decoded[to_flip] ^= 1

        # Nếu vẫn chưa hội tụ, vẫn trả về k bit đầu
        return decoded[:self.k]




def bits_to_bytes(bits: List[int]) -> List[int]:
    """
    Convert a list of bits to a list of bytes.
    
    Args:
        bits: List of bits (0s and 1s)
        
    Returns:
        List of bytes
    """
    # Ensure the number of bits is a multiple of 8
    padding = (8 - (len(bits) % 8)) % 8
    padded_bits = bits + [0] * padding
    
    # Convert every 8 bits to a byte
    bytes_output = []
    for i in range(0, len(padded_bits), 8):
        byte = 0
        for j in range(8):
            if i + j < len(padded_bits):
                byte |= (padded_bits[i + j] << (7 - j))
        bytes_output.append(byte)
        
    return bytes_output


def bytes_to_bits(bytes_input: List[int], bit_length: int = None) -> List[int]:
    """
    Convert a list of bytes to a list of bits.
    
    Args:
        bytes_input: List of bytes
        bit_length: Optional, trim the output to this number of bits
        
    Returns:
        List of bits
    """
    bits = []
    for byte in bytes_input:
        for i in range(7, -1, -1):
            bits.append((byte >> i) & 1)
            
    # Trim to specified length if needed
    if bit_length is not None:
        bits = bits[:bit_length]
        
    return bits

