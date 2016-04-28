/*
 * Evaluate a parse tree.  The value is left in the global variable
 * exitstatus.
 */

void
evaltree(union node *n, int flags)
{
        int checkexit = 0;
        void (*evalfn)(union node *, int);
        unsigned isor;
        int status;
        if (n == NULL) {
                TRACE(("evaltree(NULL) called\n"));
                goto out;
        }
#ifndef SMALL
        displayhist = 1;        /* show history substitutions done with fc */
#endif
        TRACE(("pid %d, evaltree(%p: %d, %d) called\n",
            getpid(), n, n->type, flags));
        switch (n->type) {
        default:
#ifdef DEBUG
                out1fmt("Node type = %d\n", n->type);
#ifndef USE_GLIBC_STDIO
                flushout(out1);
#endif
                break;
#endif
        case NNOT:
                evaltree(n->nnot.com, EV_TESTED);
                status = !exitstatus;
                goto setstatus;
        case NREDIR:
                errlinno = lineno = n->nredir.linno;
                if (funcline)
                        lineno -= funcline - 1;
                expredir(n->nredir.redirect);
                pushredir(n->nredir.redirect);
                status = redirectsafe(n->nredir.redirect, REDIR_PUSH);
                if (!status) {
                        evaltree(n->nredir.n, flags & EV_TESTED);
                        status = exitstatus;
                }
                if (n->nredir.redirect)
                        popredir(0);
                goto setstatus;
        case NCMD:
#ifdef notyet
                if (eflag && !(flags & EV_TESTED))
                        checkexit = ~0;
                evalcommand(n, flags, (struct backcmd *)NULL);
                break;
#else
                evalfn = evalcommand;
checkexit:
                if (eflag && !(flags & EV_TESTED))
                        checkexit = ~0;
                goto calleval;
#endif
        case NFOR:
                evalfn = evalfor;
                goto calleval;
        case NWHILE:
        case NUNTIL:
                evalfn = evalloop;
                goto calleval;
        case NSUBSHELL:
        case NBACKGND:
                evalfn = evalsubshell;
                goto checkexit;
        case NPIPE:
                evalfn = evalpipe;
                goto checkexit;
        case NCASE:
                evalfn = evalcase;
                goto calleval;
        case NAND:
        case NOR:
        case NSEMI:
#if NAND + 1 != NOR
#error NAND + 1 != NOR
#endif
#if NOR + 1 != NSEMI
#error NOR + 1 != NSEMI
#endif
                isor = n->type - NAND;
                evaltree(
                        n->nbinary.ch1,
                        (flags | ((isor >> 1) - 1)) & EV_TESTED
                );
                if (!exitstatus == isor)
                        break;
                if (!evalskip) {
                        n = n->nbinary.ch2;
evaln:
                        evalfn = evaltree;
calleval:
                        evalfn(n, flags);
                        break;
                }
                break;
        case NIF:
                evaltree(n->nif.test, EV_TESTED);
                if (evalskip)
                        break;
                if (exitstatus == 0) {
                        n = n->nif.ifpart;
                        goto evaln;
                } else if (n->nif.elsepart) {
                        n = n->nif.elsepart;
                        goto evaln;
                }
                goto success;
        case NDEFUN:
                defun(n);
success:
                status = 0;
setstatus:
                exitstatus = status;
                break;
        }
out:
        if (checkexit & exitstatus)
                goto exexit;

        if (pendingsigs)
                dotrap();

        if (flags & EV_EXIT) {
exexit:
                exraise(EXEXIT);
        }
}