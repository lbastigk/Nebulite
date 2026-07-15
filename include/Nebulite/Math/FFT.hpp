#ifndef NEBULITE_MATH_FFT_HPP
#define NEBULITE_MATH_FFT_HPP

//------------------------------------------
// Includes

// Standard Library
#include <complex>
#include <vector>

//------------------------------------------
namespace Nebulite::Math {
/**
 * @brief A class that provides static methods for performing Fast Fourier Transform (FFT) and related operations on audio data.
 * @todo Add testing!
 */
class FFT {
public:
    /**
     * @brief Computes the Fast Fourier Transform (FFT) of the given sound data.
     * @param data The sound data to transform.
     * @return A vector of complex numbers representing the FFT of the sound data.
     */
    static std::vector<std::complex<double>> fft(std::vector<double> const& data);

    /**
     * @brief Computes the Inverse Fast Fourier Transform (IFFT) of the given frequency-domain data.
     * @param X The frequency-domain data to transform back to the time domain.
     * @return A vector of complex numbers representing the IFFT of the input data, which can be converted back to audio samples.
     */
    static std::vector<std::complex<double>> fftInverse(std::vector<std::complex<double>> const& X);

    /**
     * @brief Applies a transfer function defined by the given numerator and denominator coefficients to the input sound data in the frequency domain.
     * @param data The input sound data to which the transfer function will be applied.
     * @param num The coefficients of the numerator of the transfer function, starting with the highest order.
     * @param den The coefficients of the denominator of the transfer function, starting with the highest order.
     * @return A vector of doubles representing the sound data after applying the transfer function in the frequency domain.
     * @throws std::domain_error if the leading coefficient of the denominator is zero, which is not supported.
     */
    static std::vector<double> applyTransferFunctionFrequencyDomain(std::vector<double> const& data, std::vector<double> const& num, std::vector<double> const& den);

private:
    /**
     * @brief Evaluates the transfer function defined by the given numerator and denominator coefficients at a specific angular frequency omega.
     * @param omega The angular frequency (in radians per second) at which to evaluate the transfer function.
     * @param num The coefficients of the numerator of the transfer function, starting with the highest order.
     * @param den The coefficients of the denominator of the transfer function, starting with the highest order.
     * @return A complex number representing the value of the transfer function at the specified frequency.
     */
    static std::complex<double> evalTransfer(double omega,std::vector<double> const& num,std::vector<double> const& den);
};
} // namespace Nebulite::Math
#endif // NEBULITE_MATH_FFT_HPP
