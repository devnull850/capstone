#include "factory.h"

static const char *s_access[] = {
	"UNCHANGED", "READ", "WRITE", "READ | WRITE",
};

void print_read_write_regs(char *result, csh *handle, cs_detail *detail)
{
	int i;

	if (detail->regs_read_count > 0) {
		addStr(&result, "\treading from regs: ");

		for (i = 0; i < detail->regs_read_count; ++i) {
			if (i > 0)
				addStr(&result, ", ");

			addStr(&result, "%s", cs_reg_name(*handle, detail->regs_read[i]));
		}
	}

	if (detail->regs_write_count > 0) {
		addStr(&result, "\twriting to regs: ");

		for (i = 0; i < detail->regs_write_count; ++i) {
			if (i > 0)
				addStr(&result, ", ");

			addStr(&result, "%s", cs_reg_name(*handle, detail->regs_write[i]));
		}
	}
}

char *get_detail_m680x(csh *handle, cs_mode mode, cs_insn *insn)
{
	cs_detail *detail = insn->detail;
	cs_m680x *m680x = NULL;
	int i;
	char *result;

	result = (char *)malloc(sizeof(char));
	result[0] = '\0';

	// detail can be NULL on "data" instruction if SKIPDATA option is
	// turned ON
	if (detail == NULL)
		return result;

	m680x = &detail->m680x;

	if (m680x->op_count)
		addStr(&result, " | op_count: %u", m680x->op_count);

	for (i = 0; i < m680x->op_count; i++) {
		cs_m680x_op *op = &(m680x->operands[i]);
		const char *comment;

		switch ((int)op->type) {
		default:
			break;

		case M680X_OP_REGISTER:
			comment = "";

			if ((i == 0 && m680x->flags & M680X_FIRST_OP_IN_MNEM) ||
				(i == 1 && m680x->flags &
					M680X_SECOND_OP_IN_MNEM))
				comment = " (in mnemonic)";

			addStr(&result, " | operands[%u].type: REGISTER = %s%s", i, cs_reg_name(*handle, op->reg), comment);
			break;

		case M680X_OP_CONSTANT:
			addStr(&result, " | operands[%u].type: CONSTANT = %u", i, op->const_val);
			break;

		case M680X_OP_IMMEDIATE:
			addStr(&result, " | operands[%u].type: IMMEDIATE = #%d", i, op->imm);
			break;

		case M680X_OP_DIRECT:
			addStr(&result, " | operands[%u].type: DIRECT = 0x%02X", i, op->direct_addr);
			break;

		case M680X_OP_EXTENDED:
			addStr(&result, " | operands[%u].type: EXTENDED %s = 0x%04X", i, op->ext.indirect ? "INDIRECT" : "", op->ext.address);
			break;

		case M680X_OP_RELATIVE:
			addStr(&result, " | operands[%u].type: RELATIVE = 0x%04X", i, op->rel.address);
			break;

		case M680X_OP_INDEXED:
			addStr(&result, " | operands[%u].type: INDEXED%s", i, (op->idx.flags & M680X_IDX_INDIRECT) ? " INDIRECT" : "");

			if (op->idx.base_reg != M680X_REG_INVALID)
				addStr(&result, " | base register: %s", cs_reg_name(*handle, op->idx.base_reg));

			if (op->idx.offset_reg != M680X_REG_INVALID)
				addStr(&result, " | offset register: %s", cs_reg_name(*handle, op->idx.offset_reg));

			if ((op->idx.offset_bits != 0) &&
				(op->idx.offset_reg == M680X_REG_INVALID) &&
				!op->idx.inc_dec) {
				addStr(&result, " | offset: %d", op->idx.offset);

				if (op->idx.base_reg == M680X_REG_PC)
					addStr(&result, " | offset address: 0x%X", op->idx.offset_addr);

				addStr(&result, " | offset bits: %u", op->idx.offset_bits);
			}

			if (op->idx.inc_dec) {
				const char *post_pre = op->idx.flags &
					M680X_IDX_POST_INC_DEC ? "post" : "pre";
				const char *inc_dec = (op->idx.inc_dec > 0) ?
					"increment" : "decrement";

				addStr(&result, " | %s %s: %d", post_pre, inc_dec, abs(op->idx.inc_dec));
			}

			break;
		}

		if (op->size != 0)
			addStr(&result, " | size: %u", op->size);

		if (op->access != CS_AC_INVALID)
			addStr(&result, " | access: %s", s_access[op->access]);
	}

	print_read_write_regs(result, handle, detail);
	return result;
}

