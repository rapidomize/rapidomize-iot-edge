Import("env")
import os

env.Replace(PROGNAME="rapidomize-sdk-embedded-0.7.5-b29")
#project_name = os.path.basename(os.path.dirname(env["PROJECT_CONFIG"]))
#env.Replace(PROGNAME="%s" % project_name)
