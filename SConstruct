env = Environment()
env.Append(CCFLAGS = '-std=c++11')

sub_dirs = ['common', 'config', 'database', 'traffic', 'transaction']
sub_scripts = []

for dir in sub_dirs:
    sub_scripts += ['src/' + dir + '/SConscript']
    
sub_objs = SConscript(sub_scripts)
env.Program('gMaxLinked', list(sub_objs) + ['src/gMaxLinked.cpp'], LIBS=['pthread', 'hiredis', 'log4c'])