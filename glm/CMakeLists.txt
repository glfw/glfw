set(NAME glm)

file(GLOB ROOT_SOURCE *.cpp)
file(GLOB ROOT_INLINE *.inl)
file(GLOB ROOT_HEADER *.hpp)

file(GLOB_RECURSE CORE_SOURCE ./core/*.cpp)
file(GLOB_RECURSE CORE_INLINE ./core/*.inl)
file(GLOB_RECURSE CORE_HEADER ./core/*.hpp)

file(GLOB_RECURSE GTC_SOURCE ./gtc/*.cpp)
file(GLOB_RECURSE GTC_INLINE ./gtc/*.inl)
file(GLOB_RECURSE GTC_HEADER ./gtc/*.hpp)

file(GLOB_RECURSE GTX_SOURCE ./gtx/*.cpp)
file(GLOB_RECURSE GTX_INLINE ./gtx/*.inl)
file(GLOB_RECURSE GTX_HEADER ./gtx/*.hpp)

file(GLOB_RECURSE VIRTREV_SOURCE ./virtrev/*.cpp)
file(GLOB_RECURSE VIRTREV_INLINE ./virtrev/*.inl)
file(GLOB_RECURSE VIRTREV_HEADER ./virtrev/*.hpp)

source_group("Core Files" FILES ${CORE_SOURCE})
source_group("Core Files" FILES ${CORE_INLINE})
source_group("Core Files" FILES ${CORE_HEADER})
source_group("GTC Files" FILES ${GTC_SOURCE})
source_group("GTC Files" FILES ${GTC_INLINE})
source_group("GTC Files" FILES ${GTC_HEADER})
source_group("GTX Files" FILES ${GTX_SOURCE})
source_group("GTX Files" FILES ${GTX_INLINE})
source_group("GTX Files" FILES ${GTX_HEADER})
source_group("VIRTREV Files" FILES ${VIRTREV_SOURCE})
source_group("VIRTREV Files" FILES ${VIRTREV_INLINE})
source_group("VIRTREV Files" FILES ${VIRTREV_HEADER})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(${NAME} 
	${ROOT_SOURCE}    ${ROOT_INLINE}    ${ROOT_HEADER} 
	${CORE_SOURCE}    ${CORE_INLINE}    ${CORE_HEADER}
	${GTC_SOURCE}     ${GTC_INLINE}     ${GTC_HEADER} 
	${GTX_SOURCE}     ${GTX_INLINE}     ${GTX_HEADER} 
	${VIRTREV_SOURCE} ${VIRTREV_INLINE} ${VIRTREV_HEADER})
