#ifndef QUARKGL_RANDOM_H_
#define QUARKGL_RANDOM_H_

#include <random>

namespace qrk 
{
    // A random generator for generating pseudorandom seeds.
    class SeedGenerator 
    {
    public:
        explicit SeedGenerator(unsigned int seed) : m_Gen(seed) {}

        // Returns the next sampled seed.
        unsigned int next() { return m_Gen(); }

        // Generates a single random seed using an entropy device.
        static unsigned int generateTrueRandomSeed();

    private:
        std::mt19937 m_Gen;
    };

    // A random distribution that generates uniform real numbers in the range
    // [0.0, 1.0).
    class UniformRandom 
    {
    public:
        explicit UniformRandom(unsigned int seed)
            : m_Gen(seed), m_Rand(0.0f, 1.0f), m_kuiSeed(seed) {}
        UniformRandom() : UniformRandom(SeedGenerator::generateTrueRandomSeed()) {}

        // Returns the next sampled random number.
        float next() { return m_Rand(m_Gen); }

        // Returns the seed used to initialize this random sampler.
        unsigned int getSeed() const { return m_kuiSeed; }

    private:
        // RNG for random sampling. We use the Mersenne Twister because it has
        // high-quality characteristics.
        std::mt19937 m_Gen;
        std::uniform_real_distribution<float> m_Rand;
        const unsigned int m_kuiSeed;
    };

}  // namespace qrk

#endif
