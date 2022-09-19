String.o: String/String.cpp String/String.hpp
Server.o: Server/Server.cpp Server/Server.hpp \
  Server/../DbHandler/DbHandler.hpp \
  Server/../DbHandler/../String/String.hpp \
  Server/../UserDbHandler/UserDbHandler.hpp \
  Server/../UserDbHandler/../DbHandler/DbHandler.hpp \
  Server/../FileDbHandler/FileDbHandler.hpp \
  Server/../FileDbHandler/../DbHandler/DbHandler.hpp
UserDbHandler.o: UserDbHandler/UserDbHandler.cpp \
  UserDbHandler/../DbHandler/DbHandler.hpp \
  UserDbHandler/../DbHandler/../String/String.hpp \
  UserDbHandler/UserDbHandler.hpp
FileDbHandler.o: FileDbHandler/FileDbHandler.cpp \
  FileDbHandler/FileDbHandler.hpp \
  FileDbHandler/../DbHandler/DbHandler.hpp \
  FileDbHandler/../DbHandler/../String/String.hpp
