
#include "bsp5.h"


node_t	outside_node;		// portals outside the world face this

//=============================================================================

/*
=============
AddPortalToNodes
=============
*/
void AddPortalToNodes (portal_t *p, node_t *front, node_t *back)
{
	if (p->nodes[0] || p->nodes[1])
		Error ("AddPortalToNode: allready included");

	p->nodes[0] = front;
	p->next[0] = front->portals;
	front->portals = p;
	
	p->nodes[1] = back;
	p->next[1] = back->portals;
	back->portals = p;
}


/*
=============
RemovePortalFromNode
=============
*/
void RemovePortalFromNode (portal_t *portal, node_t *l)
{
	portal_t	**pp, *t;
	
// remove reference to the current portal
	pp = &l->portals;
	while (1)
	{
		t = *pp;
		if (!t)
			Error ("RemovePortalFromNode: portal not in leaf");	

		if ( t == portal )
			break;

		if (t->nodes[0] == l)
			pp = &t->next[0];
		else if (t->nodes[1] == l)
			pp = &t->next[1];
		else
			Error ("RemovePortalFromNode: portal not bounding leaf");
	}
	
	if (portal->nodes[0] == l)
	{
		*pp = portal->next[0];
		portal->nodes[0] = NULL;
	}
	else if (portal->nodes[1] == l)
	{
		*pp = portal->next[1];	
		portal->nodes[1] = NULL;
	}
}

//============================================================================

void PrintPortal (portal_t *p)
{
	int			i;
	winding_t	*w;
	
	w = p->winding;
	for (i=0 ; i<w->numpoints ; i++)
		printf ("(%5.0f,%5.0f,%5.0f)\n",w->points[i][0]
		, w->points[i][1], w->points[i][2]);
}

/*
================
MakeHeadnodePortals

The created portals will face the global outside_node
================
*/
void MakeHeadnodePortals (node_t *node)
{
	vec3_t		bounds[2];
	int			i, j, n;
	portal_t	*p, *portals[6];
	plane_t		bplanes[6], *pl;
	int			side;
	
	Draw_ClearWindow ();
	
// pad with some space so there will never be null volume leafs
	for (i=0 ; i<3 ; i++)
	{
		bounds[0][i] = brushset->mins[i] - SIDESPACE;
		bounds[1][i] = brushset->maxs[i] + SIDESPACE;
	}
	
	outside_node.contents = CONTENTS_SOLID;
	outside_node.portals = NULL;

	for (i=0 ; i<3 ; i++)
		for (j=0 ; j<2 ; j++)
		{
			n = j*3 + i;

			p = AllocPortal ();
			portals[n] = p;
			
			pl = &bplanes[n];
			memset (pl, 0, sizeof(*pl));
			if (j)
			{
				pl->normal[i] = -1;
				pl->dist = -bounds[j][i];
			}
			else
			{
				pl->normal[i] = 1;
				pl->dist = bounds[j][i];
			}
			p->planenum = FindPlane (pl, &side);
	
			p->winding = BaseWindingForPlane (pl);
			if (side)
				AddPortalToNodes (p, &outside_node, node);
			else
				AddPortalToNodes (p, node, &outside_node);
		}
		
// clip the basewindings by all the other planes
	for (i=0 ; i<6 ; i++)
	{
		for (j=0 ; j<6 ; j++)
		{
			if (j == i)
				continue;
			portals[i]->winding = ClipWinding (portals[i]->winding, &bplanes[j], true);
		}
	}
}

//============================================================================

void CheckWindingInNode (winding_t *w, node_t *node)
{
	int		i, j;
	
	for (i=0 ; i<w->numpoints ; i++)
	{
		for (j=0 ; j<3 ; j++)
			if (w->points[i][j] < node->mins[j] - 1
			|| w->points[i][j] > node->maxs[j] + 1)
			{
				printf ("WARNING: CheckWindingInNode: outside\n");
				return;
			}
	}
}

void CheckWindingArea (winding_t *w)
{
	int		i;
	float	total, add;
	vec3_t	v1, v2, cross;
	
	total = 0;
	for (i=1 ; i<w->numpoints ; i++)
	{
		VectorSubtract (w->points[i], w->points[0], v1);
		VectorSubtract (w->points[i+1], w->points[0], v2);
		CrossProduct (v1, v2, cross);
		add = VectorLength (cross);
		total += add*0.5;
	}
	if (total < 16)
		printf ("WARNING: winding area %f\n", total);
}


void PlaneFromWinding (winding_t *w, plane_t *plane)
{
	vec3_t		v1, v2;

// calc plane
	VectorSubtract (w->points[2], w->points[1], v1);
	VectorSubtract (w->points[0], w->points[1], v2);
	CrossProduct (v2, v1, plane->normal);
	VectorNormalize (plane->normal);
	plane->dist = DotProduct (w->points[0], plane->normal);
}

void CheckLeafPortalConsistancy (node_t *node)
{
	int			side, side2;
	portal_t	*p, *p2;
	plane_t		plane, plane2;
	int			i;
	winding_t	*w;
	float		dist;

	side = side2 = 0;		// quiet compiler warning

	for (p = node->portals ; p ; p = p->next[side])	
	{
		if (p->nodes[0] == node)
			side = 0;
		else if (p->nodes[1] == node)
			side = 1;
		else
			Error ("CutNodePortals_r: mislinked portal");
		CheckWindingInNode (p->winding, node);
		CheckWindingArea (p->winding);

	// check that the side orders are correct
		plane = planes[p->planenum];
 		PlaneFromWinding (p->winding, &plane2);
		
		for (p2 = node->portals ; p2 ; p2 = p2->next[side2])	
		{
			if (p2->nodes[0] == node)
				side2 = 0;
			else if (p2->nodes[1] == node)
				side2 = 1;
			else
				Error ("CutNodePortals_r: mislinked portal");
			w = p2->winding;
			for (i=0 ; i<w->numpoints ; i++)
			{
				dist = DotProduct (w->points[i], plane.normal) - plane.dist;
				if ( (side == 0 && dist < -1) || (side == 1 && dist > 1) )
				{
					printf ("WARNING: portal siding direction is wrong\n");
					return;
				}
			}
			
		}
	}
}


/*
================
CutNodePortals_r

================
*/
static int cutnode_detail;

//static int lev=1;
void CutNodePortals_r (node_t *node)
{
	plane_t 	*plane, clipplane;
	node_t		*f, *b, *other_node;
	portal_t	*p, *new_portal, *next_portal;
	winding_t	*w, *frontwinding, *backwinding;
	int			side;

//	CheckLeafPortalConsistancy (node);

//
// seperate the portals on node into it's children	
//
//   printf("%*c",lev,' ');
	if (node->contents)
	{
//      printf("leaf\n");
		return;			// at a leaf, no more dividing
	}

   if (node->detail && cutnode_detail)
   {
//      printf("detail\n");
      return;
   }
//   printf("split\n");

	plane = &planes[node->planenum];

	f = node->children[0];
	b = node->children[1];

//
// create the new portal by taking the full plane winding for the cutting plane
// and clipping it by all of the planes from the other portals
//
	new_portal = AllocPortal ();
	new_portal->planenum = node->planenum;
	
	w = BaseWindingForPlane (&planes[node->planenum]);
	side = 0;	// shut up compiler warning
	for (p = node->portals ; p ; p = p->next[side])	
	{
		clipplane = planes[p->planenum];
		if (p->nodes[0] == node)
			side = 0;
		else if (p->nodes[1] == node)
		{
			clipplane.dist = -clipplane.dist;
			VectorSubtract (vec3_origin, clipplane.normal, clipplane.normal);
			side = 1;
		}
		else
			Error ("CutNodePortals_r: mislinked portal");

		w = ClipWinding (w, &clipplane, true);
		if (!w)
		{
			printf ("WARNING: CutNodePortals_r:new portal was clipped away\n");
			break;
		}
	}
	
	if (w)
	{
	// if the plane was not clipped on all sides, there was an error
		new_portal->winding = w;	
		AddPortalToNodes (new_portal, f, b);
	}

//
// partition the portals
//
	for (p = node->portals ; p ; p = next_portal)	
	{
		if (p->nodes[0] == node)
			side = 0;
		else if (p->nodes[1] == node)
			side = 1;
		else
			Error ("CutNodePortals_r: mislinked portal");
		next_portal = p->next[side];

		other_node = p->nodes[!side];
		RemovePortalFromNode (p, p->nodes[0]);
		RemovePortalFromNode (p, p->nodes[1]);

//
// cut the portal into two portals, one on each side of the cut plane
//
		DivideWinding (p->winding, plane, &frontwinding, &backwinding);
		
		if (!frontwinding)
		{
			if (side == 0)
				AddPortalToNodes (p, b, other_node);
			else
				AddPortalToNodes (p, other_node, b);
			continue;
		}
		if (!backwinding)
		{
			if (side == 0)
				AddPortalToNodes (p, f, other_node);
			else
				AddPortalToNodes (p, other_node, f);
			continue;
		}
		
	// the winding is split
		new_portal = AllocPortal ();
		*new_portal = *p;
		new_portal->winding = backwinding;
		FreeWinding (p->winding);
		p->winding = frontwinding;

		if (side == 0)
		{
			AddPortalToNodes (p, f, other_node);
			AddPortalToNodes (new_portal, b, other_node);
		}
		else
		{
			AddPortalToNodes (p, other_node, f);
			AddPortalToNodes (new_portal, other_node, b);
		}
	}
	
DrawLeaf (f,1);
DrawLeaf (b,2);	

//lev++;
	CutNodePortals_r (f);	
	CutNodePortals_r (b);
//lev--;
}


/*
==================
PortalizeWorld

Builds the exact polyhedrons for the nodes and leafs
==================
*/
void PortalizeWorld (node_t *headnode)
{
	qprintf ("----- portalize ----\n");

	MakeHeadnodePortals (headnode);
   cutnode_detail=0;
	CutNodePortals_r (headnode);
}


/*
==================
PortalizeWorldDetail

Like PortalizeWorld, but stop at detail nodes - Alexander Malmberg.
==================
*/
void PortalizeWorldDetail (node_t *headnode)
{
	qprintf ("----- portalize ----\n");

	MakeHeadnodePortals (headnode);
   cutnode_detail=1;
	CutNodePortals_r (headnode);
}


/*
==================
FreeAllPortals

==================
*/
void FreeAllPortals (node_t *node)
{
	portal_t	*p, *nextp;
	
	if (!node->contents)
	{
		FreeAllPortals (node->children[0]);
		FreeAllPortals (node->children[1]);
	}
	
	for (p=node->portals ; p ; p=nextp)
	{
		if (p->nodes[0] == node)
			nextp = p->next[0];
		else
			nextp = p->next[1];
		RemovePortalFromNode (p, p->nodes[0]);
		RemovePortalFromNode (p, p->nodes[1]);
		FreeWinding (p->winding);
		FreePortal (p);
	}
}

/*
==============================================================================

PORTAL FILE GENERATION

==============================================================================
*/

#define	PORTALFILE	"PRT1-AM"

FILE	*pf;
int		num_visleafs;				// leafs the player can be in
int		num_visportals;
int      num_realleafs;

void WriteFloat (FILE *f, vec_t v)
{
	if ( fabs(v - Q_rint(v)) < 0.001 )
		fprintf (f,"%i ",(int)Q_rint(v));
	else
		fprintf (f,"%f ",v);
}


/*static int lev=1;
static void PrintCont(node_t *n)
{
   printf("%*c%i\n",lev,' ',n->contents);
   if (!n->contents)
   {
      lev++;
      PrintCont(n->children[0]);
      PrintCont(n->children[1]);
      lev--;
   }
}*/

static int HasContents(node_t *n,int cont)
{
   if (n->contents==cont) return 1;
   if (n->contents) return 0;
   if (HasContents(n->children[0],cont)) return 1;
   return HasContents(n->children[1],cont);
}

static int ShareContents(node_t *n1,node_t *n2)
{
   if (n1->contents)
   {
      if (n1->contents==CONTENTS_SOLID)
         return 0;
      else
         return HasContents(n2,n1->contents);
   }

   if (ShareContents(n1->children[0],n2)) return 1;
   return ShareContents(n1->children[1],n2);
}

static int SameContents(node_t *n1,node_t *n2)
{
   if (n1->contents==CONTENTS_SOLID || n2->contents==CONTENTS_SOLID)
      return 0;

/*   printf("n1 (%i %i) n2 (%i %i)\n",
      n1->detail,n1->contents,n2->detail,n2->contents);

   printf("n1\n");
   PrintCont(n1);
   printf("n2\n");
   PrintCont(n2);*/

   if (n1->detail && n2->detail) return ShareContents(n1,n2);
   if (n1->detail) return HasContents(n1,n2->contents);
   if (n2->detail) return HasContents(n2,n1->contents);
   return n1->contents==n2->contents;
}

void WritePortalFile_r (node_t *node)
{
	int		i;	
	portal_t	*p;
	winding_t	*w;
	plane_t		*pl, plane2;

	if (!node->contents && !node->detail)
	{
		WritePortalFile_r (node->children[0]);
		WritePortalFile_r (node->children[1]);
		return;
	}
	
	if (node->contents == CONTENTS_SOLID)
		return;

	for (p = node->portals ; p ; )
	{
		w = p->winding;

/*      if (!SameContents(p->nodes[0],p->nodes[1]))
         printf("skip %i %i\n\n",p->nodes[0]->contents,p->nodes[1]->contents);*/

		if (w && p->nodes[0] == node && SameContents(p->nodes[0],p->nodes[1]))
		{
/*         if (p->nodes[1]->visleafnum==203 || p->nodes[0]->visleafnum==203)
            printf("add %i %i (%i-%i)\n\n",
               p->nodes[0]->contents,
               p->nodes[1]->contents,
               p->nodes[0]->visleafnum,
               p->nodes[1]->visleafnum);*/
		// write out to the file
		
		// sometimes planes get turned around when they are very near
		// the changeover point between different axis.  interpret the
		// plane the same way vis will, and flip the side orders if needed
			pl = &planes[p->planenum];
			PlaneFromWinding (w, &plane2);
			if ( DotProduct (pl->normal, plane2.normal) < 0.99 )
			{	// backwards...
				fprintf (pf,"%i %i %i ",w->numpoints, p->nodes[1]->visleafnum, p->nodes[0]->visleafnum);
			}
			else
				fprintf (pf,"%i %i %i ",w->numpoints, p->nodes[0]->visleafnum, p->nodes[1]->visleafnum);
			for (i=0 ; i<w->numpoints ; i++)
			{
				fprintf (pf,"(");
				WriteFloat (pf, w->points[i][0]);
				WriteFloat (pf, w->points[i][1]);
				WriteFloat (pf, w->points[i][2]);
				fprintf (pf,") ");
			}
			fprintf (pf,"\n");
		}
		
		if (p->nodes[0] == node)
			p = p->next[0];
		else
			p = p->next[1];
	}

}


void WritePortalLeafs_r(node_t *n)
{
   if (!n->contents)
   {
      WritePortalLeafs_r(n->children[0]);
      WritePortalLeafs_r(n->children[1]);
   }
   else
      if (n->visleafnum!=-1)
         fprintf(pf,"%i\n",n->visleafnum);
}

	
static void SetCluster_r(node_t *n,int num)
{
	if (n->contents == CONTENTS_SOLID)
	{	// solid block, viewpoint never inside
		n->visleafnum = -1;
		return;
	}

   n->visleafnum=num;
   if (!n->contents)
   {
      SetCluster_r(n->children[0],num);
      SetCluster_r(n->children[1],num);
   }
   else
      num_realleafs++;
}


/*
================
NumberLeafs_r
================
*/
void NumberLeafs_r (node_t *node)
{
	portal_t	*p;

	if (!node->contents && !node->detail)
	{	// decision node
		node->visleafnum = -99;
		NumberLeafs_r (node->children[0]);
		NumberLeafs_r (node->children[1]);
		return;
	}
	
	Draw_ClearWindow ();
	DrawLeaf (node, 1);
	
	if (node->contents == CONTENTS_SOLID)
	{	// solid block, viewpoint never inside
		node->visleafnum = -1;
		return;
	}

	node->visleafnum = num_visleafs++;
	
	for (p = node->portals ; p ; )
	{
		if (p->nodes[0] == node)		// only write out from first leaf
		{
			if (SameContents(p->nodes[0],p->nodes[1]))
				num_visportals++;
			p = p->next[0];
		}
		else
			p = p->next[1];		
	}

   if (node->detail)
   {
      SetCluster_r(node->children[0],node->visleafnum);
      SetCluster_r(node->children[1],node->visleafnum);
   }
   else
      num_realleafs++;
}


/*
================
WritePortalfile
================
*/
void WritePortalfile (node_t *headnode)
{
// set the visleafnum field in every leaf and count the total number of portals
	num_visleafs = 0;
	num_visportals = 0;
   num_realleafs = 0;
	NumberLeafs_r (headnode);
	
// write the file
	printf ("writing %s\n", portfilename);
	pf = fopen (portfilename, "w");
	if (!pf)
		Error ("Error opening %s", portfilename);
		
	fprintf (pf, "%s\n", PORTALFILE);
	fprintf (pf, "%i\n", num_visleafs);
	fprintf (pf, "%i\n", num_visportals);
   fprintf (pf, "%i\n", num_realleafs);
	
	WritePortalFile_r (headnode);

   WritePortalLeafs_r (headnode);
	
	fclose (pf);
}


