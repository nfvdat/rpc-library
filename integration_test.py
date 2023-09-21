#!/usr/bin/env python3

import subprocess
import atexit
import queue
import sys
import time

debugging = False

if '-debug' in sys.argv:
    debugging = True

if debugging:
    binder = subprocess.Popen('./binder.exe', stdout=subprocess.PIPE)
else:
    binder = subprocess.Popen('./binder.exe', stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)

def cleanup_binder():
    binder.kill()
    binder.wait()
atexit.register(cleanup_binder)

def get_envs(proc, env1, env2):
    val1 = None
    val2 = None
    while not proc.poll():
        line = proc.stdout.readline().decode('ascii').strip()
        prefix1 = env1 + ' '
        prefix2 = env2 + ' '
        if line.startswith(prefix1):
            val1 = line[len(prefix1):]
        if line.startswith(prefix2):
            val2 = line[len(prefix2):]
        if val1 and val2:
            break
    
    assert(val1 and val2)
    return val1, val2

binder_address, binder_port = get_envs(binder, 'BINDER_ADDRESS', 'BINDER_PORT')

# Test that the binder removes servers from its database when they leave:
# create a server, and kill it right away. It should start serving from
# the next one instead.
bad_server = subprocess.Popen('./server_example.exe', stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env={'BINDER_ADDRESS': binder_address, 'BINDER_PORT': binder_port})
# This is bad, but it'll do for now. Ensure it has time to register
time.sleep(0.1)
bad_server.kill()

if debugging:
    server = subprocess.Popen('./server_example.exe', stdout=subprocess.PIPE,
            env={'BINDER_ADDRESS': binder_address, 'BINDER_PORT': binder_port})
else:
    server = subprocess.Popen('./server_example.exe', stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env={'BINDER_ADDRESS': binder_address, 'BINDER_PORT': binder_port})

def cleanup_server():
    server.kill()
    server.wait()
atexit.register(cleanup_server)

# Delay to make sure server has time to register.
time.sleep(0.1)

if debugging:
    client = subprocess.Popen('./client_example.exe', stdout=subprocess.PIPE,
            env={'BINDER_ADDRESS': binder_address, 'BINDER_PORT': binder_port})
else:
    client = subprocess.Popen('./client_example.exe', stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env={'BINDER_ADDRESS': binder_address, 'BINDER_PORT': binder_port})

def cleanup_client():
    client.kill()
    client.wait()
atexit.register(cleanup_client)

def wait_and_assert_success(proc):
    proc.wait()
    assert(proc.returncode == 0)

wait_and_assert_success(client)
# The client should have sent termination, so we should be able to just wait
# on the binder and servers and they should finish up on their own.
wait_and_assert_success(binder)
wait_and_assert_success(server)
