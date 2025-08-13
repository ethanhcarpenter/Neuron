#include <vector>
#include <numeric>
#include <cmath>
#include <functional>
#include <algorithm>
using namespace std;

class RuleBook {
public:

    static function<vector<float>(const vector<float>&)> sumGreaterThanN(float n) {
        return [n](const vector<float>& in) -> vector<float> {
            // Correct usage of accumulate: accumulate all elements in 'in'
            float sum = accumulate(in.begin(), in.end(), 0.0f);
            float result = (sum >n) ? 1.0f : 0.0f;
            return {result};
        };
    }

    // Rule 1: Weighted sum with nonlinear threshold + variance
    static function<vector<float>(const vector<float>&)> ruleWeightedNonlinear(float threshold = 2.0f) {
        return [threshold](const vector<float>& in) -> vector<float> {
            float weightedSum = 0;
            float weight = 1.5;
            for (size_t i = 0; i < in.size(); ++i)
                weightedSum += in[i] * weight * sin(i + 1);
            float activation = (weightedSum > threshold) ? 1.0f : 0.0f;

            // Variance calculation
            float mean = weightedSum / in.size();
            float variance = 0;
            for (auto v : in) variance += (v - mean) * (v - mean);
            variance /= in.size();

            return { activation, tanh(variance) };
            };
    }

    // Rule 2: Parity with XOR of first two inputs and output interaction term
    static function<vector<float>(const vector<float>&)> ruleParityXOR() {
        return [](const vector<float>& in) -> vector<float> {
            int countOnes = 0;
            for (float v : in) if (round(v) == 1) countOnes++;

            float parity = (countOnes % 2 == 0) ? 1.0f : 0.0f;

            float xorVal = 0;
            if (in.size() >= 2) {
                int a = round(in[0]);
                int b = round(in[1]);
                xorVal = (a ^ b);
            }

            float interaction = parity * xorVal;

            return { parity, xorVal, interaction };
            };
    }

    // Rule 3: Statistical moments: mean, variance, skewness
    static function<vector<float>(const vector<float>&)> ruleStatMoments() {
        return [](const vector<float>& in) -> vector<float> {
            float n = in.size();
            float mean = accumulate(in.begin(), in.end(), 0.0f) / n;
            float m2 = 0, m3 = 0;

            for (auto x : in) {
                float diff = x - mean;
                m2 += diff * diff;
                m3 += diff * diff * diff;
            }
            m2 /= n;
            m3 /= n;
            float skewness = m3 / pow(m2, 1.5);

            return { mean, m2, skewness };
            };
    }
    //3
    static function<vector<float>(const vector<float>&)> ruleHighValueClusters(float threshold = 0.7, float clusterGap = 1) {
        return [threshold, clusterGap](const vector<float>& in) -> vector<float> {
            int n = in.size();
            if (n == 0) return { 0.0f, 0.0f, 0.0f };

            int clusterCount = 0;
            int totalClusterSize = 0;
            int currentClusterSize = 0;
            int gapCounter = clusterGap + 1; // start > clusterGap to allow cluster start

            for (int i = 0; i < n; ++i) {
                if (in[i] >= threshold) {
                    if (gapCounter > clusterGap) {
                        // start new cluster
                        clusterCount++;
                        currentClusterSize = 1;
                    }
                    else {
                        currentClusterSize++;
                    }
                    totalClusterSize += 1;
                    gapCounter = 0;
                }
                else {
                    gapCounter++;
                }
            }

            float avgClusterSize = clusterCount > 0 ? (float)totalClusterSize / clusterCount : 0.0f;
            float clusterDensity = (float)totalClusterSize / n;

            // Normalize by expected max cluster size (assume max cluster size = n)
            float normAvgClusterSize = min(1.0f, avgClusterSize / n);
            float normClusterCount = min(1.0f, clusterCount / (n / 2.0f)); // roughly max clusters ~ n/2
            float normClusterDensity = min(1.0f, clusterDensity);

            return { normClusterCount, normAvgClusterSize, normClusterDensity };
            };
    }

    //4 
    static function<vector<float>(const vector<float>&)> rulePositionalDecay() {
        return [](const vector<float>& in) -> vector<float> {
            int n = in.size();
            if (n == 0) return vector<float>(4, 0.0f);

            // Exponential decay weights from front and back
            float decayRate = 0.3;
            float weightedFrontSum = 0, weightedBackSum = 0;
            float sumWeightsFront = 0, sumWeightsBack = 0;

            for (int i = 0; i < n; ++i) {
                float wFront = exp(-decayRate * i);
                float wBack = exp(-decayRate * (n - 1 - i));
                weightedFrontSum += in[i] * wFront;
                weightedBackSum += in[i] * wBack;
                sumWeightsFront += wFront;
                sumWeightsBack += wBack;
            }

            float normFront = weightedFrontSum / (sumWeightsFront + 1e-9);
            float normBack = weightedBackSum / (sumWeightsBack + 1e-9);

            // Difference and average normalized between 0 and 1 via sigmoid
            float diff = abs(normFront - normBack);
            float avg = 0.5 * (normFront + normBack);

            auto sigmoid = [](float x) { return 1.0f / (1.0f + exp(-10 * (x - 0.5))); };
            float normDiff = sigmoid(diff);
            float normAvg = sigmoid(avg);

            return { normFront, normBack, normDiff, normAvg };
            };
    }

};

