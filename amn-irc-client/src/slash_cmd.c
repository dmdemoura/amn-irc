#include "slash_cmd.h"

#include <stdlib.h>

void SlashCmd_Delete(SlashCmd* self)
{
	free(self->param);
	free(self);
}
