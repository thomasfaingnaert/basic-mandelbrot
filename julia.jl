using CuArrays
using CUDAdrv
using CUDAnative

# Configuration
output_path = "output.ppm"
N = 1024

# Image output
function write_image(file_name::String, data::Array{UInt8, 1}, width::UInt64, height::UInt64)
    file = open(file_name, "w")

    # Header
    write(file, "P6\n")
    write(file, string(width, " ", height, "\n"))
    write(file, "255\n")

    # Data
    write(file, data)
    close(file)
end

# Complex arithmetic
struct CuComplex
    r::Float32
    i::Float32
end

CuComplex() = CuComplex(0, 0)
modulus2(z::CuComplex) = z.r * z.r + z.i * z.i
Base.:+(a::CuComplex, b::CuComplex) = CuComplex(a.r + b.r, a.i + b.i)
Base.:*(a::CuComplex, b::CuComplex) = CuComplex(a.r * b.r - a.i * b.i, a.r * b.i + a.i * b.r)

# Mandelbrot logic
function mandelbrot(x, y, N)
    z = CuComplex((x - N/2) / (N / 4), -(y - N/2) * 4/N)

    a = CuComplex()

    for i = 0:255
        a = a * a + z

        if modulus2(a) > 4
            # diverges
            return 255 - i
        end
    end

    # probably converges
    return 0
end

# Kernel and script
function kernel!(img, N)
    x = blockIdx().x - 1
    y = blockIdx().y - 1

    result = mandelbrot(x, y, N)

    @inbounds img[1 + 3 * x + 3 * N * y] = result
    @inbounds img[2 + 3 * x + 3 * N * y] = result
    @inbounds img[3 + 3 * x + 3 * N * y] = result

    return nothing
end

dev_img = CuArrays.fill(UInt8(0), 3 * N * N)

@cuda blocks=(N, N) kernel!(dev_img, N)

img = Array(dev_img)
write_image(output_path, img, UInt64(N), UInt64(N))
