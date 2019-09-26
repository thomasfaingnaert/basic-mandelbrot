#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

/*
 * Configuration
 */
static const std::string output_path = "output.ppm";
static const int N = 1024;

/*
 * Image output
 */
void write_image(const std::string& file_name, const unsigned char *data, std::size_t width, std::size_t height)
{
    std::ostringstream hdr_oss;
    hdr_oss << "P6\n";
    hdr_oss << width << " " << height << "\n";
    hdr_oss << 255 << "\n";

    std::string hdr = hdr_oss.str();

    std::ofstream ofs(file_name);
    ofs.write(hdr.data(), hdr.size());
    ofs.write(reinterpret_cast<const char*>(data), width * height * 3);
}

/*
 * Complex arithmetic
 */
struct cu_complex
{
    float r; // real part
    float i; // imaginary part

    cu_complex() : r{0}, i{0} { }
    cu_complex(float r, float i) : r{r}, i{i} { }

    float modulus2() { return r * r + i * i; }
};

cu_complex operator+(const cu_complex &a, const cu_complex &b)
{
    return cu_complex{a.r + b.r, a.i + b.i};
}

cu_complex operator*(const cu_complex &a, const cu_complex &b)
{
    return cu_complex{a.r * b.r - a.i * b.i, a.r * b.i + a.i * b.r};
}

/*
 * Mandelbrot logic
 */
int mandelbrot(int x, int y)
{
    cu_complex z{static_cast<float>(x - N/2) / (N / 4), -static_cast<float>(y - N/2) * 4/N};

    cu_complex a;

    for (int i = 0; i < 255; ++i)
    {
        a = a * a + z;

        if (a.modulus2() > 4)
        {
            // diverges
            return 255 - i;
        }
    }

    // probably converges
    return 0;
}

/*
 * Kernel and main function
 */
void kernel(int x, int y, unsigned char *img)
{
    const int result = mandelbrot(x, y);

    img[0 + 3 * x + 3 * N * y] = result;
    img[1 + 3 * x + 3 * N * y] = result;
    img[2 + 3 * x + 3 * N * y] = result;
}

int main(int argc, char **argv)
{
    unsigned char *img = new unsigned char[3 * N * N];

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            kernel(i, j, img);
        }
    }

    write_image(output_path, img, N, N);
    delete[] img;
}
