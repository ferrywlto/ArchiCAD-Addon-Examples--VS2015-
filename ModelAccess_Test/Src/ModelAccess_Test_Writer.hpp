// *****************************************************************************
// Write ModelAccess Test files
// API Development Kit 19; Mac/Win
//
// Namespaces:		Contact person:
//	   WRL
//
// [SG compatible] - Yes
// *****************************************************************************

#ifndef	_MATWRITER_HPP_
#define	_MATWRITER_HPP_

//----------------------------------- Includes ---------------------------------

#include "APIEnvir.h"
#include "ACAPinc.h"

#include "Collection.hpp"

//-------------------------------- Predeclarations -----------------------------

namespace ModelerAPI {
	class Model;
	class TextureCoordinate;
}

namespace IO {
	class File;
}

//--------------------------------- Namespace WRL ------------------------------

namespace WRL {

//------------------------------- Class definitions ----------------------------

//--------------------------------- Class MAWriter -----------------------------

class MAWriter {
private:
		// data members

	ModelerAPI::Model*		model;
	IO::File*				outputFile;
	bool*					definedMaterials;
	Int32					nMaterials;

public:

		// constructor / destructor

	MAWriter (const IO::Location& ioParam_fileLoc, ModelerAPI::Model* maModel);
   ~MAWriter ();

		// header writer methods

	void		WriteHeader (char* str);
	void		WriteNavigationInfo (void);

		// material / texture writer methods

	void		WriteDefMaterial (Int32 iumat);
	void		WriteTexCoordBegin (void);
	void		WriteTexCoordEnd (void);
	void		WriteTexCoordAdd (bool isLast, ModelerAPI::TextureCoordinate* uvCoord);
	void		WriteTexCoordIndexBegin (void);
	void		WriteTexCoordIndexEnd (void);
	void		WriteTexCoordIndexListAdd (bool isLast, Int32 coordInd);
	void		WriteAppearance (Int32 iumat, char* textureName);

		// lightsource writer methods

	void		WriteDirectionalLight (double rc, double gc, double bc, double dirx, double diry, double dirz);
	void		WriteSpotLight (double rc, double gc, double bc, double posx, double posy, double posz,
								double dirx, double diry, double dirz, double cutOffAngle);
	void		WritePointLight (double intensity, double rc, double gc, double bc,	double posx, double posy, double posz);

		// group writer methods

	void		WriteGroupBegin (void);
	void		WriteGroupEnd (void);
	void		WriteShapeBegin (void);
	void		WriteShapeEnd (void);

		// vertex writer methods

	void		WriteCoordinateBegin (bool first);
	void		WriteCoordinateEnd (bool first);
	void		WritePoint (bool isLast, double x, double y, double z, Int32 index);

		// polygon writer methods

	void		WriteIndexedFaceSetBegin (bool solid);
	void		WriteIndexedFaceSetEnd (void);
	void		WriteCoordIndexBegin (void);
	void		WriteCoordIndexEnd (void);
	void		WriteCoordIndexListAdd (bool isLast, Int32 coordInd);

		// normal writer methods

	void		WriteNormalBegin (bool first);
	void		WriteNormalEnd (bool first);
	void		WriteVectorListAdd (bool isLast, Int32 index, double nx, double ny, double nz);
	void		WriteNormalIndexBegin (void);
	void		WriteNormalIndexEnd (void);
	void		WriteNormalIndexListAdd (bool isLast, Int32 normalInd);

		// camera writer methods

	void		WriteViewpoint (Int32 i, double posx, double posy, double posz, double orx, double ory, double orz, double amount);

private:
		// buffer writer methods

	void		WriteBuffer (char* buffer);

};

//--------------------------------- Namespace WRL ------------------------------

}

#endif
