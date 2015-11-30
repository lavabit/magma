#!/bin/bash

echo ""
tput setaf 6; echo "DKIM Signature Verification:"; tput sgr0
echo ""
printf "EHLO localhost
MAIL FROM: <ladar.levison@gmail.com>
RCPT TO: <ladar@lavabit.com>
DATA
Return-Path: <ladar.levison@gmail.com>
Received: from mail-pv0-f171.google.com (74.125.83.171)
        by lavabit.com with ESMTP id SLL28RS0POAA
        for <ladar@lavabit.com>; Wed, 13 Apr 2011 13:10:02 -0500
Received: by pva4 with SMTP id 4so451063pva.16
        for <ladar@lavabit.com>; Wed, 13 Apr 2011 11:09:26 -0700 (PDT)
DKIM-Signature: v=1; a=rsa-sha256; c=relaxed/relaxed;
        d=gmail.com; s=gamma;
        h=domainkey-signature:mime-version:in-reply-to:references:date
         :message-id:subject:from:to:content-type;
        bh=w8VSrgExO0HK5iy2aCjzi42G9PsjpULiGxVJqqTo/og=;
        b=s9nHIPCRuLneQoXnmQWSw5wM3j6F9YWtf/BYIQVvirn6FrLnulEJDpCB75C2tK1/GF
         5bTBo3DB5r4hgRPbp7M7vG6Ex4XsOe3aeiKi3dt+Eqy1Nazg+SgE8jKdfH9Bi7MT0UyG
         TAH8Tyl1hLsZzkg3M2lmNz8KIog1zLSkMX0/g=
DomainKey-Signature: a=rsa-sha1; c=nofws;
        d=gmail.com; s=gamma;
        h=mime-version:in-reply-to:references:date:message-id:subject:from:to
         :content-type;
        b=X+VCba1OaaXOCZ9LmW6ZgFmlx+jvpYV2ol9ty9+t/BtQ2PlCPAC08XZsLztL9Ky/4S
         Qe0jR54km7bRamkTCIR/FVK4YRHBMDzvVB0zUgs0YRzNPAhsnZDdO2dhL58vNYPk1hXM
         JVYs44S6O9Rya045PPBUDCfwypQo6sUwj5TWU=
MIME-Version: 1.0
Received: by 10.142.151.41 with SMTP id y41mr7308823wfd.94.1302718166118; Wed,
 13 Apr 2011 11:09:26 -0700 (PDT)
Received: by 10.142.224.11 with HTTP; Wed, 13 Apr 2011 11:09:26 -0700 (PDT)
In-Reply-To: <BANLkTinB8mkGG5Ssk_pTrFBo4JHCTJM3KA@mail.gmail.com>
References: <BANLkTinB8mkGG5Ssk_pTrFBo4JHCTJM3KA@mail.gmail.com>
Date: Wed, 13 Apr 2011 13:09:26 -0500
Message-ID: <BANLkTinsOGF+MY_Py6+zSjOSnJV=RhvXSg@mail.gmail.com>
Subject: Re: Hello world of authenticated email!
From: Ladar Levison <ladar.levison@gmail.com>
To: Ladar Levison <ladar@lavabit.com>
Content-Type: text/plain; charset=ISO-8859-1

Part two!

On Wed, Apr 13, 2011 at 1:04 PM, Ladar Levison <ladar.levison@gmail.com> wrote:
> Can I join the party?
>
.
QUIT
" | nc localhost 7000