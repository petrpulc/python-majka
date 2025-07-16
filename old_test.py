import majka

@profile
def my_func():
  m = majka.Majka('/home/pulc/Projekty/linker/cs.w-lt')
  for i in range(1,10000):
    x = m.find('hloupý')
  del m

if __name__ == '__main__':
  my_func()
