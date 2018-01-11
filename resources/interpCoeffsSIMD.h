/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2017 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at
 
 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#ifndef interpCoeffsSIMD_h
 #define interpCoeffsSIMD_h
#define interpLength 4
#define interpOffset 1
#define interpShift 7
#define interpMult 128
#define interpMask 127


const __m128 interpCoeffsSIMD[129] = {{0.0, 1.0, 0.0, 0.0, },
    {-0.0025737, 0.99603, 0.0078428, -0.001302, },
    {-0.0050869, 0.99195, 0.015745, -0.0026035, },
    {-0.00754, 0.98774, 0.023706, -0.0039041, },
    {-0.0099335, 0.98341, 0.031723, -0.0052032, },
    {-0.012268, 0.97897, 0.039796, -0.0065005, },
    {-0.014544, 0.97442, 0.047922, -0.0077953, },
    {-0.016761, 0.96975, 0.056101, -0.0090873, },
    {-0.018921, 0.96497, 0.064331, -0.010376, },
    {-0.021024, 0.96007, 0.072611, -0.011661, },
    {-0.023069, 0.95507, 0.080938, -0.012941, },
    {-0.025059, 0.94996, 0.089313, -0.014217, },
    {-0.026993, 0.94475, 0.097733, -0.015488, },
    {-0.028871, 0.93943, 0.1062, -0.016752, },
    {-0.030695, 0.934, 0.1147, -0.018011, },
    {-0.032464, 0.92848, 0.12325, -0.019263, },
    {-0.03418, 0.92285, 0.13184, -0.020508, },
    {-0.035842, 0.91713, 0.14046, -0.021745, },
    {-0.037451, 0.9113, 0.14912, -0.022974, },
    {-0.039007, 0.90538, 0.15782, -0.024194, },
    {-0.040512, 0.89937, 0.16655, -0.025406, },
    {-0.041965, 0.89326, 0.17531, -0.026608, },
    {-0.043367, 0.88706, 0.18411, -0.0278, },
    {-0.044719, 0.88077, 0.19293, -0.028981, },
    {-0.046021, 0.87439, 0.20178, -0.030151, },
    {-0.047272, 0.86792, 0.21066, -0.03131, },
    {-0.048475, 0.86137, 0.21956, -0.032457, },
    {-0.049629, 0.85473, 0.22849, -0.033592, },
    {-0.050735, 0.84801, 0.23744, -0.034714, },
    {-0.051794, 0.8412, 0.24641, -0.035822, },
    {-0.052805, 0.83432, 0.2554, -0.036917, },
    {-0.053769, 0.82735, 0.26441, -0.037997, },
    {-0.054688, 0.82031, 0.27344, -0.039063, },
    {-0.05556, 0.81319, 0.28248, -0.040113, },
    {-0.056387, 0.806, 0.29153, -0.041147, },
    {-0.057169, 0.79874, 0.3006, -0.042166, },
    {-0.057907, 0.7914, 0.30968, -0.043167, },
    {-0.058601, 0.78399, 0.31876, -0.044152, },
    {-0.059252, 0.77651, 0.32786, -0.045118, },
    {-0.05986, 0.76896, 0.33696, -0.046067, },
    {-0.060425, 0.76135, 0.34607, -0.046997, },
    {-0.060948, 0.75368, 0.35518, -0.047908, },
    {-0.06143, 0.74594, 0.36429, -0.0488, },
    {-0.061871, 0.73813, 0.37341, -0.049671, },
    {-0.062271, 0.73027, 0.38252, -0.050522, },
    {-0.062631, 0.72235, 0.39163, -0.051352, },
    {-0.062952, 0.71437, 0.40074, -0.05216, },
    {-0.063234, 0.70633, 0.40985, -0.052947, },
    {-0.063477, 0.69824, 0.41895, -0.053711, },
    {-0.063681, 0.6901, 0.42804, -0.054452, },
    {-0.063848, 0.6819, 0.43712, -0.05517, },
    {-0.063978, 0.67366, 0.44619, -0.055864, },
    {-0.064072, 0.66536, 0.45525, -0.056534, },
    {-0.064129, 0.65702, 0.46429, -0.057179, },
    {-0.06415, 0.64863, 0.47332, -0.057798, },
    {-0.064136, 0.64019, 0.48234, -0.058392, },
    {-0.064087, 0.63171, 0.49133, -0.05896, },
    {-0.064004, 0.62319, 0.50031, -0.059501, },
    {-0.063887, 0.61463, 0.50927, -0.060015, },
    {-0.063736, 0.60603, 0.5182, -0.060501, },
    {-0.063553, 0.5974, 0.52711, -0.060959, },
    {-0.063337, 0.58872, 0.536, -0.061388, },
    {-0.063089, 0.58002, 0.54486, -0.061789, },
    {-0.06281, 0.57127, 0.5537, -0.062159, },
    {-0.0625, 0.5625, 0.5625, -0.0625, },
    {-0.062159, 0.5537, 0.57127, -0.06281, },
    {-0.061789, 0.54486, 0.58002, -0.063089, },
    {-0.061388, 0.536, 0.58872, -0.063337, },
    {-0.060959, 0.52711, 0.5974, -0.063553, },
    {-0.060501, 0.5182, 0.60603, -0.063736, },
    {-0.060015, 0.50927, 0.61463, -0.063887, },
    {-0.059501, 0.50031, 0.62319, -0.064004, },
    {-0.05896, 0.49133, 0.63171, -0.064087, },
    {-0.058392, 0.48234, 0.64019, -0.064136, },
    {-0.057798, 0.47332, 0.64863, -0.06415, },
    {-0.057179, 0.46429, 0.65702, -0.064129, },
    {-0.056534, 0.45525, 0.66536, -0.064072, },
    {-0.055864, 0.44619, 0.67366, -0.063978, },
    {-0.05517, 0.43712, 0.6819, -0.063848, },
    {-0.054452, 0.42804, 0.6901, -0.063681, },
    {-0.053711, 0.41895, 0.69824, -0.063477, },
    {-0.052947, 0.40985, 0.70633, -0.063234, },
    {-0.05216, 0.40074, 0.71437, -0.062952, },
    {-0.051352, 0.39163, 0.72235, -0.062631, },
    {-0.050522, 0.38252, 0.73027, -0.062271, },
    {-0.049671, 0.37341, 0.73813, -0.061871, },
    {-0.0488, 0.36429, 0.74594, -0.06143, },
    {-0.047908, 0.35518, 0.75368, -0.060948, },
    {-0.046997, 0.34607, 0.76135, -0.060425, },
    {-0.046067, 0.33696, 0.76896, -0.05986, },
    {-0.045118, 0.32786, 0.77651, -0.059252, },
    {-0.044152, 0.31876, 0.78399, -0.058601, },
    {-0.043167, 0.30968, 0.7914, -0.057907, },
    {-0.042166, 0.3006, 0.79874, -0.057169, },
    {-0.041147, 0.29153, 0.806, -0.056387, },
    {-0.040113, 0.28248, 0.81319, -0.05556, },
    {-0.039063, 0.27344, 0.82031, -0.054687, },
    {-0.037997, 0.26441, 0.82735, -0.053769, },
    {-0.036917, 0.2554, 0.83432, -0.052805, },
    {-0.035822, 0.24641, 0.8412, -0.051794, },
    {-0.034714, 0.23744, 0.84801, -0.050735, },
    {-0.033592, 0.22849, 0.85473, -0.049629, },
    {-0.032457, 0.21956, 0.86137, -0.048475, },
    {-0.03131, 0.21066, 0.86792, -0.047272, },
    {-0.030151, 0.20178, 0.87439, -0.046021, },
    {-0.028981, 0.19293, 0.88077, -0.044719, },
    {-0.0278, 0.18411, 0.88706, -0.043367, },
    {-0.026608, 0.17531, 0.89326, -0.041965, },
    {-0.025406, 0.16655, 0.89937, -0.040512, },
    {-0.024194, 0.15782, 0.90538, -0.039007, },
    {-0.022974, 0.14912, 0.9113, -0.037451, },
    {-0.021745, 0.14046, 0.91713, -0.035842, },
    {-0.020508, 0.13184, 0.92285, -0.03418, },
    {-0.019263, 0.12325, 0.92848, -0.032464, },
    {-0.018011, 0.1147, 0.934, -0.030695, },
    {-0.016752, 0.1062, 0.93943, -0.028871, },
    {-0.015488, 0.097733, 0.94475, -0.026993, },
    {-0.014217, 0.089313, 0.94996, -0.025059, },
    {-0.012941, 0.080938, 0.95507, -0.023069, },
    {-0.011661, 0.072611, 0.96007, -0.021024, },
    {-0.010376, 0.064331, 0.96497, -0.018921, },
    {-0.0090873, 0.056101, 0.96975, -0.016761, },
    {-0.0077953, 0.047922, 0.97442, -0.014544, },
    {-0.0065005, 0.039796, 0.97897, -0.012268, },
    {-0.0052032, 0.031723, 0.98341, -0.0099335, },
    {-0.0039041, 0.023706, 0.98774, -0.00754, },
    {-0.0026035, 0.015745, 0.99195, -0.0050869, },
    {-0.001302, 0.0078428, 0.99603, -0.0025737, },
    {0.0, 0.0 , 1.0, 0.0, }
};

// idx is the 7-bit-quantized fraction of your sample delay time , whereas fraction yields the fraction between two consecutive idx-values //TODO: write this text more understandable

__m128 inline getInterpolatedLagrangeWeights(int idx, float fraction) {
    __m128 lowPart = _mm_set1_ps(1.0f - fraction);
    __m128 highPart = _mm_set1_ps(fraction);
    __m128 interp = _mm_add_ps (_mm_mul_ps(lowPart, interpCoeffsSIMD[idx]), _mm_mul_ps(highPart, interpCoeffsSIMD[idx+1]));
    return interp;
}

#endif
