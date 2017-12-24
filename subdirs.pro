TEMPLATE = subdirs

CONFIG += ordered

TARGET = TransportationProblem

win32:VERSION = 1.0.0.0 # major.minor.patch.build
else:VERSION = 1.0.0    # major.minor.patch

SUBDIRS = core app
