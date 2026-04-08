import spidev
import time
import math

SAMPLE_COUNT = 500
VREF = 5.0          # Updated to match sensor max voltage
ADC_MAX = 1023.0
CALIBRATION_TIME = 60
SAMPLE_DELAY = 0.002   # 2 ms delay between samples

LEARNING_RATE = 0.0001
TOL = 1e-5
MAX_ITERS = 1000
VOLTAGE_DIVIDER = 2.0
CURRENT_SENSOR_V_PER_A = 0.1

# SPI setup
spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 1350000

def read_adc(channel):
    cmd = [1, (8 + channel) << 4, 0]
    resp = spi.xfer2(cmd)
    return ((resp[1] & 3) << 8) | resp[2]

def compute_rms(samples, offset=0.0):
    sum_sq = sum((v - offset) ** 2 for v in samples)
    return math.sqrt(sum_sq / len(samples))

def sample_adc(channel, sample_count=SAMPLE_COUNT, delay=SAMPLE_DELAY):
    samples = []
    for _ in range(sample_count):
        samples.append((read_adc(channel) / ADC_MAX) * VREF)
        time.sleep(delay)
    return samples

def gradient_descent_offset(samples, init_offset, learning_rate=LEARNING_RATE, tol=TOL, max_iters=MAX_ITERS):
    offset = init_offset
    for _ in range(max_iters):
        grad = -2 * sum((v - offset) for v in samples) / len(samples)
        offset_new = offset - learning_rate * grad
        if abs(offset_new - offset) < tol:
            break
        offset = offset_new
    return offset

def calibrate_sensors(known_voltage):
    print("Step 1: Measure zero readings with mains OFF")
    voltage_samples_off = sample_adc(1)
    current_samples_off = sample_adc(0)

    voltage_offset = gradient_descent_offset(voltage_samples_off, sum(voltage_samples_off)/len(voltage_samples_off))
    current_offset = gradient_descent_offset(current_samples_off, sum(current_samples_off)/len(current_samples_off))

    print(f"Zero voltage ADC -> {voltage_offset:.4f} V")
    print(f"Zero current ADC -> {current_offset:.4f} V")

    print("\nStep 2: WAIT 5 SECONDS before applying mains")
    time.sleep(5)

    input("Step 3: Turn mains ON / apply known load then press Enter...")

    voltage_samples_on = sample_adc(1)
    current_samples_on = sample_adc(0)

    voltage_rms = compute_rms(voltage_samples_on, voltage_offset)
    current_rms = compute_rms(current_samples_on, current_offset)

    # Apply voltage divider correction
    voltage_gain = (known_voltage / voltage_rms) * VOLTAGE_DIVIDER
    current_scale = VOLTAGE_DIVIDER
    current_sensitivity = CURRENT_SENSOR_V_PER_A

    print("\nCalibration complete! Use these constants in C code:")
    print(f"voltageOffset = {voltage_offset:.4f} V")
    print(f"voltageGain   = {voltage_gain:.4f}")
    print(f"currentOffset = {current_offset:.4f} V")
    print(f"currentScale  = {current_scale:.4f}")
    print(f"currentSens   = {current_sensitivity:.4f} V/A")

    return voltage_offset, voltage_gain, current_offset, current_scale, current_sensitivity

if __name__ == "__main__":
    calibrate_sensors(known_voltage=120.0)
