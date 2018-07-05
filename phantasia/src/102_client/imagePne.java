import java.awt.*;

public class imagePne extends Canvas implements constants {

    pClient parent = null;
    Image theImage = null;

    public imagePne(pClient c) {

	super();
	parent = c;
	setBackground(highlightColor);
    }

    public void paint(Graphics g) {

	if (theImage != null) {
	    g.drawImage(theImage, 0, 0, this);
	}
    }

    public void setImage(String passedImage) {

        MediaTracker mt = new MediaTracker(this);

	theImage = parent.getImage(parent.getCodeBase(), passedImage);
	mt.addImage(theImage, 0);

	try {
	    mt.waitForAll();
	}
	catch (InterruptedException e) {};

	repaint();
    }
}

