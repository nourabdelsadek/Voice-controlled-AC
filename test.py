import serial
import time
import sys

# --- CONFIGURATION ---
COM_PORT = 'COM6'  # Make sure this matches your Arduino
BAUD_RATE = 9600

def parse_serial_line(line):
    """Parses the CSV line from Arduino: State, Target, In, Out, Speed"""
    try:
        parts = line.split(',')
        if len(parts) == 5:
            return {
                "state": parts[0],
                "target": int(parts[1]),
                "in": int(parts[2]),
                "out": int(parts[3]),
                "speed": int(parts[4])
            }
    except ValueError:
        pass
    return None

def wait_for_message(ser, expected_msg, timeout=10):
    """Waits for a specific exact message (like 'Start' or 'Stop')"""
    start_time = time.time()
    print(f"   [Waiting for '{expected_msg}'...]")
    while time.time() - start_time < timeout:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line == expected_msg:
                return True
    return False

def get_stable_reading(ser):
    """Clears buffer and gets a fresh reading line"""
    ser.reset_input_buffer()
    # Read a few lines to skip old data
    for _ in range(5):
        ser.readline() 
    
    # Try to get a valid parsed line
    for _ in range(10):
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        data = parse_serial_line(line)
        if data:
            return data
    return None

def send_target_temp(ser, temperature):
    """Sends the temperature using the specific formatted protocol requested"""
    to_send = f"{temperature}\n"
    if len(to_send) == 2:
        to_send = '0' + to_send
    
    print(f"   [Sending bytes: {repr(to_send)}]") # Debug print to confirm format
    ser.write(to_send.encode())

def run_tests():
    tests_run = 0
    tests_passed = 0

    print(f"--- Connecting to {COM_PORT} ---")
    try:
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=2)
        time.sleep(2) # Wait for Arduino restart
        print("Connected!\n")
    except Exception as e:
        print(f"Error connecting: {e}")
        sys.exit(1)

    # ================= TEST 1: BUTTON START =================
    tests_run += 1
    print("\nTEST 1: Button 'Start' Logic")
    input(">>> ACTION REQUIRED: Press the Arduino button to START. (Press Enter to continue script)")
    
    if wait_for_message(ser, "Start"):
        print("✅ PASS: Received 'Start' command.")
        tests_passed += 1
    else:
        print("❌ FAIL: Did not receive 'Start' in time.")

    # ================= TEST 2: BUTTON STOP =================
    tests_run += 1
    print("\nTEST 2: Button 'Stop' Logic")
    input(">>> ACTION REQUIRED: Press the Arduino button to STOP. (Press Enter to continue script)")
    
    if wait_for_message(ser, "Stop"):
        print("✅ PASS: Received 'Stop' command.")
        tests_passed += 1
    else:
        print("❌ FAIL: Did not receive 'Stop' in time.")

    # ================= TEST 3: COMMUNICATION CHECK =================
    tests_run += 1
    print("\nTEST 3: Target Transmission Check (Sending '20')")
    
    # --- MODIFIED SENDING LOGIC ---
    send_target_temp(ser, 20)
    time.sleep(1) # Give Arduino time to process
    
    data = get_stable_reading(ser)
    if data and data['target'] == 20:
        print(f"✅ PASS: Arduino echoed Target: {data['target']}")
        tests_passed += 1
    else:
        print(f"❌ FAIL: Expected 20, got {data['target'] if data else 'None'}")

    input(">>> ACTION REQUIRED: Verify LCD shows Target: 20. (Press Enter if YES)")

    # ================= TEST 4: STOP LOGIC (Difference = 1) =================
    tests_run += 1
    print("\nTEST 4: Hysteresis Logic (Target = Inner - 1)")
    # Get current inner temp first
    current_state = get_stable_reading(ser)
    if not current_state:
        print("❌ FAIL: Could not read sensor data.")
        return # Exit early, stats will be printed for tests run so far? No, return exits function.
        # To print stats on failure, we'd need try/finally, but standard behavior is to stop.

    curr_in = current_state['in']
    test_target = curr_in - 1
    
    print(f"   Setting Target to {test_target} (Current Inner: {curr_in})")
    
    # --- MODIFIED SENDING LOGIC ---
    send_target_temp(ser, test_target)
    time.sleep(1)
    
    new_state = get_stable_reading(ser)
    if new_state['state'] == 'S':
        print(f"✅ PASS: Motor State is 'S' (Stopped) as expected (Diff is 1).")
        tests_passed += 1
    else:
        print(f"❌ FAIL: Expected 'S', got '{new_state['state']}'")


    # ================= TEST 5: COOLING LOGIC (Target = Inner - 20) =================
    tests_run += 1
    print("\nTEST 5: Cooling Logic (Target = Inner - 20)")
    curr_in = new_state['in']
    curr_out = new_state['out']
    test_target = max(0, curr_in - 20) # Avoid negative
    
    print(f"   Setting Target to {test_target} (Current In: {curr_in}, Out: {curr_out})")
    
    # --- MODIFIED SENDING LOGIC ---
    send_target_temp(ser, test_target)
    time.sleep(1)
    
    final_state = get_stable_reading(ser)
    
    # Determine EXPECTED logic based on your C code
    expected_motor = '?'
    if curr_in > test_target: # Need Cool
        if curr_out < curr_in:
            expected_motor = 'F' # Fan (cool air in)
            desc = "Forward (Cool outside air in)"
        else:
            expected_motor = 'B' # Hood (Hot inside air out)
            desc = "Backward (Exhaust mode)"
            
    if final_state['state'] == expected_motor:
        print(f"✅ PASS: Motor State is '{final_state['state']}' ({desc}).")
        tests_passed += 1
    else:
        print(f"❌ FAIL: Expected '{expected_motor}', got '{final_state['state']}'")

    input(f">>> ACTION REQUIRED: Check LCD. Does it show Motor: {desc}? (Press Enter)")


    # ================= TEST 6: HEATING LOGIC (Target = Inner + 20) =================
    tests_run += 1
    print("\nTEST 6: Heating Logic (Target = Inner + 20)")
    curr_in = final_state['in']
    curr_out = final_state['out']
    test_target = min(99, curr_in + 20)
    
    print(f"   Setting Target to {test_target} (Current In: {curr_in}, Out: {curr_out})")
    
    # --- MODIFIED SENDING LOGIC ---
    send_target_temp(ser, test_target)
    time.sleep(1)
    
    final_state = get_stable_reading(ser)
    
    # EXPECTED Logic
    expected_motor = '?'
    if curr_in < test_target: # Need Heat
        if curr_out > curr_in:
            expected_motor = 'F' # Fan (Warm air in)
            desc = "Forward (Warm outside air in)"
        else:
            expected_motor = 'S' # Stop (Don't bring in cold air)
            desc = "Stopped (Outside is too cold)"

    if final_state['state'] == expected_motor:
        print(f"✅ PASS: Motor State is '{final_state['state']}' ({desc}).")
        tests_passed += 1
    else:
        print(f"❌ FAIL: Expected '{expected_motor}', got '{final_state['state']}'")


    # ================= TEST 7: SENSOR HEAT CHECK (OUTSIDE) =================
    tests_run += 1
    print("\nTEST 7A: Outside Sensor Reactivity")
    
    # --- OUTSIDE SENSOR ---
    initial_reading = get_stable_reading(ser)
    start_temp = initial_reading['out']
    print(f"   Current Outside Temp: {start_temp}°C")
    print(">>> ACTION REQUIRED: Hold/Heat the OUTSIDE sensor for 5 seconds.")
    
    detected_rise = False
    for i in range(10): # Check for 10 seconds
        data = get_stable_reading(ser)
        print(f"   Reading: {data['out']}°C...", end="\r")
        if data['out'] > start_temp + 2: # Threshold of 2 degrees
            detected_rise = True
            print(f"\n✅ PASS: Outside temp rose to {data['out']}°C!")
            tests_passed += 1
            break
        time.sleep(1)
        
    if not detected_rise:
        print("\n⚠️ WARNING: Outside temp did not rise significantly.")

    # ================= TEST 7B: SENSOR HEAT CHECK (INSIDE) =================
    tests_run += 1
    print("\nTEST 7B: Inside Sensor Reactivity")
    
    # --- INSIDE SENSOR ---
    initial_reading = get_stable_reading(ser)
    start_temp = initial_reading['in']
    print(f"   Current Inside Temp: {start_temp}°C")
    print(">>> ACTION REQUIRED: Hold/Heat the INSIDE sensor for 5 seconds.")
    
    detected_rise = False
    for i in range(10):
        data = get_stable_reading(ser)
        print(f"   Reading: {data['in']}°C...", end="\r")
        if data['in'] > start_temp + 2:
            detected_rise = True
            print(f"\n✅ PASS: Inside temp rose to {data['in']}°C!")
            tests_passed += 1
            break
        time.sleep(1)

    if not detected_rise:
        print("\n⚠️ WARNING: Inside temp did not rise significantly.")

    # ================= FINAL REPORT =================
    print("\n" + "="*30)
    print("       TEST SUMMARY       ")
    print("="*30)
    print(f"Tests Run:    {tests_run}")
    print(f"Tests Passed: {tests_passed}")
    print(f"Tests Failed: {tests_run - tests_passed}")
    
    if tests_run > 0:
        score = (tests_passed / tests_run) * 100
        print(f"Success Rate: {score:.1f}%")
        
        if score == 100:
            print("\n🌟 EXCELLENT: System passed all checks! 🌟")
        elif score >= 80:
            print("\n👍 GOOD: System is mostly functional.")
        else:
            print("\n⚠️ ATTENTION: Debugging required.")
    else:
        print("No tests were completed.")
    print("="*30)

    ser.close()

if __name__ == "__main__":
    run_tests()