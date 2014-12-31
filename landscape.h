struct landscape {
	struct GLvarray vtx;
	struct GLbuffer tri;
};

void landscape_create(struct landscape *);
void landscape_draw(struct landscape *);
