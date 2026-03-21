Import("env")
import os

# print("Current build flags:", env.get("BUILD_FLAGS")) # Note: env.get() is a safer way to access

# my_flags = env.ParseFlags(env['BUILD_FLAGS'])
# defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}
# print(defines)

env.Replace(PROGNAME="rapidomize-iot-edge-0.7.5-b32")
#project_name = os.path.basename(os.path.dirname(env["PROJECT_CONFIG"]))
# rpz_version = env.get("BUILD_FLAGS").get("RPZ_VERSION")
# env.Replace(PROGNAME="rapidomize-iot-edge-%s" % defines.get("RPZ_VERSION"))
