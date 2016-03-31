penteensy - USB HID for Penetration Testing
===========================================

This device is used on pentests where we have physical access to an unlocked device either through social engineering or sneaking in. It types quicker and with less errors than a human could, allowing testers to get in and out very quickly.

Each DIP switch corresponds to a specific payload:
- Switch #1 - Netcat - Connects out to the attacker machine via a named pipe
- Switch #2 - Powershell - Uses Metasploit's Powershell web delivery
- Switch #3 - Python - Uses the Python meterpreter payload to get a shell.
- Switch #4 - Test - Verifies the device is working (only on a Mac for simplicity)

The Teensy is plugged into the target system via USB, the desired payload runs, and an initial presence is gained. Ideally, another tester is on the attacker system receiving the callbacks and establishing persistence.

More info:
https://matterpreter.com/penteesy/

To-Do:
- [] Clean up code formatting
- [] Find a better way of setting the variable for the Python shell
- [] Provide the option to have the target download files
