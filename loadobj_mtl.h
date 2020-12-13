#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <GL/gl.h>

#include <vector>


// delimiters for parsing the obj file:

#define OBJDELIMS		" \t"
const void  *	NULLPTR = (void *)0;
const char *	DELIMS = " \t";
float White[] = { 1.,1.,1.,1. };

// utility to create an array from 3 separate values:
float*
Array4(float a, float b, float c, float d)
{
	static float array[4];
	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = d;
	return array;
}

int
Readline( FILE *fp, char *line )
{
	char *cp = line;
	int c;
	int numChars;

	while( ( c = fgetc(fp) ) != '\n'  &&  c != EOF )
	{
		if( c == '\r' )
		{
			continue;
		}
		//fprintf( stderr, "c = '%c' (0x%02x)\n", c, c );
		*cp++ = c;
	}
	*cp = '\0';

	// reject an empty line:

	numChars = 0;
	char cc;
	for( char *cp = line; ( cc = *cp ) != '\0'; cp++ )
	{
		if( cc != '\t'  &&  cc != ' ' )
			numChars++;
	}
	if( numChars == 0 )
	{
		line[0] = '\0';
	}

	return c;	// the character that caused the while-loop to terminate
}


class
Mtl
{
    public:
	char		* Name;
	float		Ka[3];				//ambient color
	float		Kd[3];				//diffuse color
	float		Ke[3];				//emission
	float		Ks[3];				//specular color
	float		D;					//transparent
	float		Ni;					//optical density (ignored)
	float		Ns;					//specular exponent (0 to 1000)
	float		Illum;				//illumination models (ignored)
	char		* MapKd;			//texture map .tga file (ignord)
	int			valid[6];
	Mtl			* Next;

	Mtl( )
	{
		Next = (Mtl *) NULLPTR;
		for (int i = 0; i < 6; i++)
			valid[i] = 0;
	};

};


class
Mtls
{
    public:
	FILE *		Fp;
	Mtl *		First;
	Mtl *		Last;

	Mtls( )
	{
		Fp = (FILE *) NULLPTR;
		First = Last = (Mtl *) NULLPTR;
	};

	int			// 0 = success, !0 = failure
	Open( char * fileName )
	{
		Fp = fopen( fileName, "r" );
		if( Fp == NULLPTR )
		{
			fprintf( stderr, "Cannot open file '%s'\n", fileName );
			return 1;
		}

		return 0;
	};

	void
	ReadMtlFile( )
	{
		char line[256];
		Mtl * thisMtl = (Mtl *) NULLPTR;
		int c;
		while( ( c = Readline( Fp, line) ) != EOF )
		{
			if( c == EOF )
				break;

			if( line[0] == '\0' )
				continue;

			//fprintf( stderr, "line = '%s'\n", line );

			char * tok = strtok( line, DELIMS );			// first token is the variable name
			char cc;
			for( char *cp = tok; ( cc = *cp ) != '\0'; cp++ )
			{
				if( isupper(cc) )
					*cp = tolower( cc );
			}
			//fprintf( stderr, "Variable name = '%s'\n", tok );

			// process the rest of the line:

			while( tok != (char *) NULLPTR )
			{
				if( tok[0] == '#' )
					break;		// comment causes us to stop reading this line
				//fprintf( stderr, "tok = *%s*\n", tok );

				if( strcmp( tok, "newmtl" ) == 0 )
				{
					thisMtl = new Mtl;
					thisMtl->Next = (Mtl *) NULLPTR;
					if( First == (Mtl *) NULLPTR )
					{
						First = thisMtl;
					}
					if( Last != (Mtl *) NULLPTR )
					{
						Last->Next = thisMtl;
					}
					Last = thisMtl;
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Name = strdup(tok);
					//fprintf( stderr, "Material name = '%s'\n", thisMtl->Name );
					break;
				}

				if( strcmp( tok, "ka" ) == 0 )
				{
					thisMtl->valid[0] = 1;
					tok = strtok( (char *)NULLPTR, DELIMS );
					if( tok != (char *)NULLPTR )	thisMtl->Ka[0] = atof( tok );
					tok = strtok( (char *)NULLPTR, DELIMS );
					if( tok != (char *)NULLPTR )	thisMtl->Ka[1] = atof( tok );
					tok = strtok( (char *)NULLPTR, DELIMS );
					if( tok != (char *)NULLPTR )	thisMtl->Ka[2] = atof( tok );
					break;
				}

				if( strcmp( tok, "kd" ) == 0 )
				{
					thisMtl->valid[1] = 1;
					tok = strtok( (char *)NULLPTR, DELIMS );
					if( tok != (char *)NULLPTR )	thisMtl->Kd[0] = atof( tok );
					tok = strtok( (char *)NULLPTR, DELIMS );
					if( tok != (char *)NULLPTR )	thisMtl->Kd[1] = atof( tok );
					tok = strtok( (char *)NULLPTR, DELIMS );
					if( tok != (char *)NULLPTR )	thisMtl->Kd[2] = atof( tok );
					break;
				}

				if( strcmp( tok, "ke" ) == 0 )
				{
					thisMtl->valid[2] = 1;
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Ke[0] = atof( tok );
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Ke[1] = atof( tok );
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Ke[2] = atof( tok );
					break;
				}

				if( strcmp( tok, "ks" ) == 0 )
				{
					thisMtl->valid[3] = 1;
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Ks[0] = atof( tok );
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Ks[1] = atof( tok );
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Ks[2] = atof( tok );
					break;
				}

				if( strcmp( tok, "d" ) == 0 )
				{
					thisMtl->valid[4] = 1;
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->D = atof( tok );
					break;
				}

				if( strcmp( tok, "ni" ) == 0 )
				{
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Ni = atof( tok );
					break;
				}

				if( strcmp( tok, "ns" ) == 0 )
				{
					thisMtl->valid[5] = 1;
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Ns = atof( tok );
					break;
				}

				if( strcmp( tok, "illum" ) == 0 )
				{
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->Illum = atof( tok );
					break;
				}

				if( strcmp( tok, "map_kd" ) == 0 )
				{
					tok = strtok( (char *)NULLPTR, DELIMS );
					thisMtl->MapKd = strdup( tok );
					break;
				}

				fprintf( stderr, "Don't recognize Mtl file token '%s'\n", tok );
			}
		}
	};

	void
	Close( )
	{
		fclose( Fp );
		Fp = (FILE *)NULLPTR;
	};

	Mtl *
	FindMtl( char *mtlName )
	{
		for( Mtl * mtl = First; mtl != (Mtl *)NULLPTR; mtl = mtl->Next )
		{
			if( strcmp( mtl->Name, mtlName ) == 0 )
			{
				return mtl;
			}
		}
		return (Mtl *)NULLPTR;
	};

};


struct Vertex
{
	float x, y, z;
};


struct Normal
{
	float nx, ny, nz;
};


struct TextureCoord
{
	float s, t, p;
};


struct face
{
	int v, n, t;
};



void	Cross( float [3], float [3], float [3] );
char *	ReadRestOfLine( FILE * );
void	ReadObjVTN( char *, int *, int *, int * );
float	Unit( float [3] );
float	Unit( float [3], float [3] );


int
LoadObjFile( char *name )
{
	char *cmd;					// the command string
	char *str;					// argument string
	char mtlFilename[50]; 		//the mtl filename string

	int load_mtl = 0;
	int applied = 1;

	std::vector <struct Vertex> Vertices(10000);
	std::vector <struct Normal> Normals(10000);
	std::vector <struct TextureCoord> TextureCoords(10000);

	Vertices.clear();
	Normals.clear();
	TextureCoords.clear();

	struct Vertex sv;
	struct Normal sn;
	struct TextureCoord st;

	Mtls myMaterials;
	Mtl* blinnMtl;

	// open the input file:

	FILE *fp = fopen( name, "r" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open .obj file '%s'\n", name );
		return 1;
	}


	float xmin = 1.e+37f;
	float ymin = 1.e+37f;
	float zmin = 1.e+37f;
	float xmax = -xmin;
	float ymax = -ymin;
	float zmax = -zmin;

	glBegin( GL_TRIANGLES );

	for( ; ; )
	{
		char *line = ReadRestOfLine( fp );
		if( line == NULL )
			break;


		// skip this line if it is a comment:

		if( line[0] == '#' )
			continue;


		// skip this line if it is something we don't feel like handling today:

		if( line[0] == 'g' )
			continue;

		if( line[0] == 's' )
			continue;


		// get the command string:

		cmd = strtok( line, OBJDELIMS );


		// skip this line if it is empty:

		if( cmd == NULL )
			continue;

		if( strcmp( cmd, "mtllib" ) == 0 )
		{
			str = strtok( NULL, OBJDELIMS );
			strcpy(mtlFilename, str);
			//fprintf(stderr, "mtl file: %s\n", mtlFilename);
			if (myMaterials.Open(mtlFilename) != 0)
				fprintf(stderr, "Could not read the mtl file\n");
			else
			{
				myMaterials.ReadMtlFile();
				myMaterials.Close();
				load_mtl = 1;
			}
		}

		if (strcmp(cmd, "usemtl") == 0 && load_mtl)
		{
			str = strtok(NULL, OBJDELIMS);
			blinnMtl = myMaterials.FindMtl(str);
			applied = 0;
		}

		if( strcmp( cmd, "v" )  ==  0 )
		{
			str = strtok( NULL, OBJDELIMS );
			sv.x = atof(str);

			str = strtok( NULL, OBJDELIMS );
			sv.y = atof(str);

			str = strtok( NULL, OBJDELIMS );
			sv.z = atof(str);

			Vertices.push_back( sv );

			if( sv.x < xmin )	xmin = sv.x;
			if( sv.x > xmax )	xmax = sv.x;
			if( sv.y < ymin )	ymin = sv.y;
			if( sv.y > ymax )	ymax = sv.y;
			if( sv.z < zmin )	zmin = sv.z;
			if( sv.z > zmax )	zmax = sv.z;

			continue;
		}


		if( strcmp( cmd, "vn" )  ==  0 )
		{
			str = strtok( NULL, OBJDELIMS );
			sn.nx = atof( str );

			str = strtok( NULL, OBJDELIMS );
			sn.ny = atof( str );

			str = strtok( NULL, OBJDELIMS );
			sn.nz = atof( str );

			Normals.push_back( sn );

			continue;
		}


		if( strcmp( cmd, "vt" )  ==  0 )
		{
			st.s = st.t = st.p = 0.;

			str = strtok( NULL, OBJDELIMS );
			st.s = atof( str );

			str = strtok( NULL, OBJDELIMS );
			if( str != NULL )
				st.t = atof( str );

			str = strtok( NULL, OBJDELIMS );
			if( str != NULL )
				st.p = atof( str );

			TextureCoords.push_back( st );

			continue;
		}


		if( strcmp( cmd, "f" )  ==  0 )
		{
			struct face vertices[10];
			for( int i = 0; i < 10; i++ )
			{
				vertices[i].v = 0;
				vertices[i].n = 0;
				vertices[i].t = 0;
			}

			int sizev = (int)Vertices.size();
			int sizen = (int)Normals.size();
			int sizet = (int)TextureCoords.size();

			int numVertices = 0;
			bool valid = true;
			int vtx = 0;
			char *str;
			while( ( str = strtok( NULL, OBJDELIMS ) )  !=  NULL )
			{
				int v, n, t;
				ReadObjVTN( str, &v, &t, &n );

				// if v, n, or t are negative, they are wrt the end of their respective list:

				if( v < 0 )
					v += ( sizev + 1 );

				if( n < 0 )
					n += ( sizen + 1 );

				if( t < 0 )
					t += ( sizet + 1 );


				// be sure we are not out-of-bounds (<vector> will abort):

				if( t > sizet )
				{
					if( t != 0 )
						fprintf( stderr, "Read texture coord %d, but only have %d so far\n", t, sizet );
					t = 0;
				}

				if( n > sizen )
				{
					if( n != 0 )
						fprintf( stderr, "Read normal %d, but only have %d so far\n", n, sizen );
					n = 0;
				}

				if( v > sizev )
				{
					if( v != 0 )
						fprintf( stderr, "Read vertex coord %d, but only have %d so far\n", v, sizev );
					v = 0;
					valid = false;
				}

				vertices[vtx].v = v;
				vertices[vtx].n = n;
				vertices[vtx].t = t;
				vtx++;

				if( vtx >= 10 )
					break;

				numVertices++;
			}


			// if vertices are invalid, don't draw anything this time:

			if( ! valid )
				continue;

			if( numVertices < 3 )
				continue;

			if (blinnMtl != (Mtl*)NULLPTR && applied == 0)
			{
				applied = 1;
				float alpha = 1.;
				if (blinnMtl->valid[4] == 1)
					alpha = blinnMtl->D;

				if (blinnMtl->valid[0] == 1)
					glMaterialfv(GL_FRONT, GL_AMBIENT,
						Array4(blinnMtl->Ka[0], blinnMtl->Ka[1], blinnMtl->Ka[2],
							alpha));

				if (blinnMtl->valid[1] == 1)
					glMaterialfv(GL_FRONT, GL_DIFFUSE,
						Array4(blinnMtl->Kd[0], blinnMtl->Kd[1], blinnMtl->Kd[2],
							1.));

				if (blinnMtl->valid[2] == 1)
					glMaterialfv(GL_FRONT, GL_EMISSION,
						Array4(blinnMtl->Ke[0], blinnMtl->Ke[1], blinnMtl->Ke[2],
							1.));

				if (blinnMtl->valid[3] == 1)
					glMaterialfv(GL_FRONT, GL_SPECULAR,
						Array4(blinnMtl->Ks[0], blinnMtl->Ks[1], blinnMtl->Ks[2],
							alpha));

				if (blinnMtl->valid[5] == 1)
				{
					float shinness = blinnMtl->Ns / 1000. * 128.;
					glMaterialf(GL_FRONT, GL_SHININESS, shinness);
				}
				glColor4f(blinnMtl->Ka[0], blinnMtl->Ka[1], blinnMtl->Ka[2], alpha);
			}

			// list the vertices:

			int numTriangles = numVertices - 2;

			for( int it = 0; it < numTriangles; it++ )
			{
				int vv[3];
				vv[0] = 0;
				vv[1] = it + 1;
				vv[2] = it + 2;

				// get the planar normal, in case vertex normals are not defined:

				struct Vertex *v0 = &Vertices[ vertices[ vv[0] ].v - 1 ];
				struct Vertex *v1 = &Vertices[ vertices[ vv[1] ].v - 1 ];
				struct Vertex *v2 = &Vertices[ vertices[ vv[2] ].v - 1 ];

				float v01[3], v02[3], norm[3];
				v01[0] = v1->x - v0->x;
				v01[1] = v1->y - v0->y;
				v01[2] = v1->z - v0->z;
				v02[0] = v2->x - v0->x;
				v02[1] = v2->y - v0->y;
				v02[2] = v2->z - v0->z;
				Cross( v01, v02, norm );
				Unit( norm, norm );
				glNormal3fv( norm );

				for( int vtx = 0; vtx < 3 ; vtx++ )
				{
					if( vertices[ vv[vtx] ].t != 0 )
					{
						struct TextureCoord *tp = &TextureCoords[ vertices[ vv[vtx] ].t - 1 ];
						glTexCoord2f( tp->s, tp->t );
					}

					if( vertices[ vv[vtx] ].n != 0 )
					{
						struct Normal *np = &Normals[ vertices[ vv[vtx] ].n - 1 ];
						glNormal3f( np->nx, np->ny, np->nz );
					}

					struct Vertex *vp = &Vertices[ vertices[ vv[vtx] ].v - 1 ];

					glVertex3f( vp->x, vp->y, vp->z );


				}
			}
			continue;
		}


		if( strcmp( cmd, "s" )  ==  0 )
		{
			continue;
		}

	}

	glEnd();
	fclose( fp );

	//fprintf( stderr, "Obj file range: [%8.3f,%8.3f,%8.3f] -> [%8.3f,%8.3f,%8.3f]\n",
		//xmin, ymin, zmin,  xmax, ymax, zmax );
	//fprintf( stderr, "Obj file center = (%8.3f,%8.3f,%8.3f)\n",
		//(xmin+xmax)/2., (ymin+ymax)/2., (zmin+zmax)/2. );
	//fprintf( stderr, "Obj file  span = (%8.3f,%8.3f,%8.3f)\n",
		//xmax-xmin, ymax-ymin, zmax-zmin );

	return 0;
}



void
Cross( float v1[3], float v2[3], float vout[3] )
{
	float tmp[3];

	tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
	tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
	tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
Unit( float v[3] )
{
	float dist;

	dist = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

	if( dist > 0.0 )
	{
		dist = sqrt( dist );
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}

	return dist;
}



float
Unit( float vin[3], float vout[3] )
{
	float dist;

	dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];

	if( dist > 0.0 )
	{
		dist = sqrt( dist );
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}


char *
ReadRestOfLine( FILE *fp )
{
	static char *line;
	std::vector<char> tmp(1000);
	tmp.clear();

	for( ; ; )
	{
		int c = getc( fp );

		if( c == EOF  &&  tmp.size() == 0 )
		{
			return NULL;
		}

		if( c == EOF  ||  c == '\n' )
		{
			delete [] line;
			line = new char [ tmp.size()+1 ];
			for( int i = 0; i < (int)tmp.size(); i++ )
			{
				line[i] = tmp[i];
			}
			line[ tmp.size() ] = '\0';	// terminating null
			return line;
		}
		else
		{
			tmp.push_back( c );
		}
	}

	return "";
}


void
ReadObjVTN( char *str, int *v, int *t, int *n )
{
	// can be one of v, v//n, v/t, v/t/n:

	if( strstr( str, "//") )				// v//n
	{
		*t = 0;
		sscanf( str, "%d//%d", v, n );
		return;
	}
	else if( sscanf( str, "%d/%d/%d", v, t, n ) == 3 )	// v/t/n
	{
		return;
	}
	else
	{
		*n = 0;
		if( sscanf( str, "%d/%d", v, t ) == 2 )		// v/t
		{
			return;
		}
		else						// v
		{
			*n = *t = 0;
			sscanf( str, "%d", v );
		}
	}
}
